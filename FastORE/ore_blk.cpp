#include "ore_blk.h"
#include "errors.h"
#include "flags.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define DEBUG_INFO 0

const bool ORE_NO_MERGE = false;

// Helper macro for error handling
static int _error_flag;
#define ERR_CHECK(x) if((_error_flag = x) != ERROR_NONE) { return _error_flag; }

// The ceiling function
#define CEIL(x, y) (((x) + (y) - 1) / (y))


// The ORE encryption scheme is randomized, so the randomness used for
// encryption is derived from an internal PRG (implemented using AES in
// counter mode). This is for demo purposes only. For concrete applications,
// it may be preferable to use a different source for the encryption
// randomness. Note that this implementation is NOT thread-safe, and is
// intended primarily for demo purposes.
static bool _prg_initialized = false;
static uint64_t _counter = 0;
static ORE_AES_KEY _prg_key;

// The maximum supported block length in bite (chosen primarily for efficiency
// reasons).
static const int MAX_BLOCK_LEN = 16;
static const int MAX_CMP_LEN = 64;


static bool ore_is_valid_params(ore_params params)
{
	if (!params->initialized) {
		return false;
	}
	else if (params->block_len == 0 || params->block_len > MAX_BLOCK_LEN ||
		params->cmp_len > MAX_CMP_LEN) {
		return false;
	}

	return true;
}

int init_ore_params(ore_params params, uint32_t nbits, uint32_t block_len, uint32_t cmp_len)
{
	params->initialized = true;
	params->nbits = nbits;
	params->block_len = block_len;
	params->cmp_len = cmp_len;

	if (!ore_is_valid_params(params)) {
		return ERROR_PARAMS_INVALID;
	}

	return ERROR_NONE;
}

int ore_key_setup(char* input, ore_key& sk, ore_params params)
{
	if (!ore_is_valid_params(params)) {
		return ERROR_PARAMS_INVALID;
	}
	block left;
	block right;
	block middle;

	memset(sk->org_key, 0, sizeof(sk->org_key));
	strcpy(sk->org_key, input);

	byte* res256 = malloc(SHA256_OUTPUT_BYTES);
	sha_256(res256, SHA256_OUTPUT_BYTES, (byte*)sk->org_key, sizeof(sk->org_key));

	memcpy(&left, res256, AES_BLOCK_LEN);
	memcpy(&right, res256 + AES_BLOCK_LEN, AES_BLOCK_LEN);
	memcpy(&middle, res256 + AES_BLOCK_LEN / 2, AES_BLOCK_LEN);

	ERR_CHECK(setup_aes_key(&sk->prf_key, (byte*)&left, AES_BLOCK_LEN));
	ERR_CHECK(setup_aes_key(&sk->prp_key, (byte*)&right, AES_BLOCK_LEN));
	ERR_CHECK(setup_aes_key(&sk->prg_key, (byte*)&middle, AES_BLOCK_LEN));

	sk->prg_counter = 0;

	memcpy(sk->params, params, sizeof(ore_params));

	sk->initialized = true;

	free(res256);

	return ERROR_NONE;
}

int ore_key_cleanup(ore_key& sk)
{
	memset(sk, 0, sizeof(ore_key));

	return ERROR_NONE;
}

static int next_prg_block(block* out, ore_key& sk) {
	return aes_eval(out, &sk->prg_key, MAKE_BLOCK(0, sk->prg_counter++));
}

static int bit_compare(const byte* byte1, const byte* byte2, uint16_t n)
{
	for (int i = 0; i < n; ++i)
	{
		if (*(byte1++) != *(byte2++))
			return *(byte1 - 1) > *(byte2 - 1) ? ORE_LARGE : ORE_SMALL;
	}
	return ORE_EQUAL;
}

static int block_prp_loc(uint64_t* block_loc, ORE_AES_KEY& sk, uint32_t nblocks, bool inv)
{
	prp_eval_all(block_loc, &sk, 8);

	int curLoc = 0;
	for (int i = 0; i < 256; i++)
	{
		if (block_loc[i] < nblocks)
		{
			block_loc[curLoc++] = block_loc[i];
		}
	}

	if(inv)
	{
		uint64_t * tmp_loc = malloc(sizeof(uint64_t)* nblocks);
		memcpy(tmp_loc, block_loc, sizeof(uint64_t)* nblocks);

		int invLoc = 0;
		for (int i = 0; i < nblocks; i++)
		{
			for (int j = 0; j < nblocks; j++)
			{
				if (tmp_loc[j] == invLoc)
				{
					block_loc[i] = j;
					invLoc++;
					break;
				}
			}
		}
		free(tmp_loc);
	}

}

static int ore_setup_helper(ore_index ctxt, ore_query cque, ore_key& sk,
	uint64_t msg_min, uint64_t msg_max, uint8_t aim)
{
	bool isquery = ctxt == 0 ? true : false;

	if (!sk->initialized)
		return ERROR_SK_NOT_INITIALIZED;

	if (isquery)
	{
		if (cque == NULL)
			return ERROR_NULL_POINTER;

		if (!cque->initialized)
			return ERROR_CTXT_NOT_INITIALIZED;
	}
	else
	{
		if (ctxt == NULL)
			return ERROR_NULL_POINTER;

		if (!ctxt->initialized)
			return ERROR_CTXT_NOT_INITIALIZED;
	}

	if (!ore_is_valid_params(sk->params))
		return ERROR_PARAMS_INVALID;

	ore_params params;
	memcpy(params, sk->params, sizeof(ore_params));

	uint32_t nbits = params->nbits;
	uint32_t block_len = params->block_len;
	uint32_t nslots = 1 << block_len;
	uint32_t nblocks = CEIL(nbits, block_len);
	uint32_t cmp_len = CEIL(params->cmp_len, 8);

	byte* comp_left = 0;
	byte* comp_right = 0;

	uint32_t len_left_block = AES_BLOCK_LEN;
	uint32_t len_right_block = cmp_len*nslots * 2 + sizeof(ORE_MERGE);

	uint64_t block_mask = (1 << block_len) - 1;
	block_mask <<= (block_len * (nblocks - 1));

	block nonce;
	uint64_t* block_loc = malloc(sizeof(uint64_t) * (1 << 8));
	if (isquery)
	{
		comp_left = cque->comp_left;

		//for (int i = 0; i < (1 << 8); i++)
		//{
		//	block_loc[i] = i;
		//}
	}
	else
	{
		comp_right = ctxt->comp_right + AES_BLOCK_LEN;
		ERR_CHECK(next_prg_block(&nonce, sk));
		memcpy(ctxt->comp_right, &nonce, AES_BLOCK_LEN);

		////use block_loc to change block order
		//ORE_AES_KEY loc_key;
		//ERR_CHECK(setup_aes_key(&loc_key, (byte*)&nonce, sizeof(block)));
		//block_prp_loc(block_loc, loc_key, nblocks, false);

	}

	block_prp_loc(block_loc, sk->prp_key, nblocks, false);

	if (DEBUG_INFO)
	{
		printf("nblocks:%d\n", nblocks);
		printf("blkLoc enc: ");
		for (int i = 0; i < nblocks; i++)
		{
			printf("%d ", block_loc[i]);
		}
		printf("\n");
	}

	uint32_t mergeTag = 0;
	uint32_t realRightSize = AES_BLOCK_LEN;
	uint64_t prefix = 0;
	uint64_t prefix2 = 0;


	block* inputs = malloc(sizeof(block) * nslots);
	block* inputs_cmp = malloc(sizeof(block) * nslots);
	uint64_t* pi_inv = malloc(sizeof(uint64_t) * nslots);
	uint64_t* pi_inv2 = malloc(sizeof(uint64_t) * nslots);

	block prp_key_buf;
	ORE_AES_KEY prp_key;
	ORE_AES_KEY prp_key2;

	ORE_AES_KEY cmp_key_nonce;
	ERR_CHECK(setup_aes_key(&cmp_key_nonce, (byte*)&nonce, AES_KEY_BYTES));

	for (int i = 0; i < nblocks; i++) {
		// compute blk_len value
		uint64_t cur_block = msg_min & block_mask;
		cur_block >>= block_len * (nblocks - i - 1);

		uint64_t cur_block2 = msg_max & block_mask;
		cur_block2 >>= block_len * (nblocks - i - 1);

		block_mask >>= block_len;

		//init block`s prp_Key
		ERR_CHECK(aes_eval(&prp_key_buf, &sk->prp_key, MAKE_BLOCK(i, prefix)));
		ERR_CHECK(setup_aes_key(&prp_key, (byte*)&prp_key_buf, sizeof(block)));

		ERR_CHECK(aes_eval(&prp_key_buf, &sk->prp_key, MAKE_BLOCK(i, prefix2)));
		ERR_CHECK(setup_aes_key(&prp_key2, (byte*)&prp_key_buf, sizeof(block)));

		if (isquery)
		{
			uint64_t pix = 0;
			ERR_CHECK(prp_eval((byte*)&pix, &prp_key, (byte*)&cur_block, block_len));
			block key;
			uint64_t prefix_shifted = prefix << block_len;
			ERR_CHECK(aes_eval(&key, &sk->prf_key, MAKE_BLOCK(i << (8 * sizeof(aim)) | aim, prefix_shifted | pix)));
			memcpy(comp_left + len_left_block * block_loc[i], &key, AES_BLOCK_LEN);
			
			if (DEBUG_INFO)
			{
				printf("Left %d: %.16llx\n", i, prefix_shifted);
			}
			
		}
		else
		{
			uint64_t prefix_shifted = prefix << block_len;
			uint64_t prefix_shifted2 = prefix2 << block_len;
			if (DEBUG_INFO)
			{
				printf("Right1 %d: %.16llx\n", i, prefix_shifted);
				printf("Right2 %d: %.16llx\n", i, prefix_shifted2);
			}

			uint64_t right_buffer;

			ERR_CHECK(prp_inv_eval_all(pi_inv, &prp_key, block_len));
			ERR_CHECK(prp_inv_eval_all(pi_inv2, &prp_key2, block_len));

			if (prefix == prefix2 && !ORE_NO_MERGE)
			{
				mergeTag = ORE_MERGE;
				memcpy(comp_right + len_right_block * block_loc[i], &mergeTag, sizeof(mergeTag));

				realRightSize += sizeof(mergeTag) + cmp_len * nslots;

				if (DEBUG_INFO)
				{
					printf("enc at block %d-->%d, mergeTag %d, offset_right %d .\n", i, block_loc[i], mergeTag,comp_right + len_right_block * block_loc[i] - ctxt->comp_right);
				}
				for (int j = 0; j < nslots; j++)
				{
					uint8_t v1 = (pi_inv[j] == cur_block) ? ORE_EQUAL : ((pi_inv[j] < cur_block) ? ORE_SMALL : ORE_LARGE);
					uint8_t v2 = (pi_inv2[j] == cur_block2) ? ORE_EQUAL : ((pi_inv2[j] < cur_block2) ? ORE_SMALL : ORE_LARGE);

					uint8_t v;
					uint8_t u;
					if (v1 == ORE_LARGE && v2 == ORE_SMALL)
					{
						u = ORE_ZERO;
						v = v1;
					}
					else if (v1 == ORE_LARGE && v2 != ORE_SMALL)
					{
						u = ORE_ONE;
						v = v1;
					}
					else if (v1 != ORE_LARGE && v2 == ORE_SMALL)
					{
						u = ORE_TWO;
						v = v2;
					}
					else
					{
						u = ORE_PADDING;
						v = ORE_EQUAL;
					}

					inputs[j] = MAKE_BLOCK(i << (8 * sizeof(v)) | v, prefix_shifted | j);

					ERR_CHECK(aes_eval(&inputs_cmp[j], &sk->prf_key, inputs[j]));
					ERR_CHECK(aes_eval(&inputs[j], &cmp_key_nonce, inputs_cmp[j]));

					memcpy(&right_buffer, &inputs[j], cmp_len);

					right_buffer += u;

					memcpy(comp_right + len_right_block * block_loc[i] + sizeof(mergeTag) + (cmp_len)*j, &right_buffer, cmp_len);
				}
			}
			else
			{
				mergeTag = ORE_NOT_MERGE;
				memcpy(comp_right + len_right_block * block_loc[i], &mergeTag, sizeof(mergeTag));

				realRightSize += sizeof(mergeTag) + cmp_len * nslots * 2;

				if (DEBUG_INFO)
				{
					printf("enc at block %d-->%d, mergeTag %d, offset_right %d .\n", i, block_loc[i], mergeTag, comp_right + len_right_block * block_loc[i] - ctxt->comp_right);
				}
				for (int j = 0; j < nslots; j++)
				{
					uint8_t v1 = (pi_inv[j] == cur_block) ? ORE_EQUAL : ((pi_inv[j] < cur_block) ? ORE_SMALL : ORE_LARGE);

					uint8_t v;
					uint8_t u;
					if (v1 == ORE_LARGE)
					{
						u = ORE_ONE;
						v = v1;
					}
					else
					{
						u = ORE_PADDING;
						v = -1;
					}

					inputs[j] = MAKE_BLOCK(i << (8 * sizeof(v)) | v, prefix_shifted | j);

					ERR_CHECK(aes_eval(&inputs_cmp[j], &sk->prf_key, inputs[j]));
					ERR_CHECK(aes_eval(&inputs[j], &cmp_key_nonce, inputs_cmp[j]));

					memcpy(&right_buffer, &inputs[j], cmp_len);

					right_buffer += u;

					memcpy(comp_right + len_right_block * block_loc[i] + sizeof(mergeTag) + (cmp_len)*j, &right_buffer, cmp_len);
				}

				for (int j = 0; j < nslots; j++)
				{
					uint8_t v2 = (pi_inv2[j] == cur_block2) ? ORE_EQUAL : ((pi_inv2[j] < cur_block2) ? ORE_SMALL : ORE_LARGE);

					uint8_t v;
					uint8_t u;
					if (v2 == ORE_SMALL)
					{
						u = ORE_TWO;
						v = v2;
					}
					else
					{
						u = ORE_PADDING;
						v = -1;
					}

					inputs[j] = MAKE_BLOCK(i << (8 * sizeof(v)) | v, prefix_shifted2 | j);

					ERR_CHECK(aes_eval(&inputs_cmp[j], &sk->prf_key, inputs[j]));
					ERR_CHECK(aes_eval(&inputs[j], &cmp_key_nonce, inputs_cmp[j]));

					memcpy(&right_buffer, &inputs[j], cmp_len);

					right_buffer += u;

					memcpy(comp_right + len_right_block * block_loc[i] + sizeof(mergeTag) + cmp_len * nslots + (cmp_len)*j, &right_buffer, cmp_len);
				}
			}
		}

		// update prefix
		prefix <<= block_len;
		prefix |= cur_block;

		prefix2 <<= block_len;
		prefix2 |= cur_block2;

	}


	if (!isquery)
	{
		memcpy(ctxt->comp_right + ore_index_right_len(params) - sizeof(realRightSize), &realRightSize,sizeof(realRightSize));
	}

	free(pi_inv);
	free(pi_inv2);
	free(inputs);
	free(inputs_cmp);

	free(block_loc);

	return ERROR_NONE;
}

int ore_index_setup(ore_index ctxt, ore_key& sk, uint64_t min, uint64_t max)
{
	return ore_setup_helper(ctxt, 0, sk, min, max, 0);
}

int ore_query_setup(ore_query ctxt, ore_key& sk, uint64_t msg, uint8_t aim)
{
	return ore_setup_helper(0, ctxt, sk, msg, 0, aim);
}

int ore_query_left_len(ore_params params)
{
	uint32_t nblocks = CEIL(params->nbits, params->block_len);
	return (AES_BLOCK_LEN)* nblocks;
}

int ore_index_right_len(ore_params params)
{
	uint32_t block_len = params->block_len;
	uint32_t nslots = (1 << block_len) * 2;
	uint32_t nblocks = CEIL(params->nbits, block_len);
	uint32_t cmp_len = CEIL(params->cmp_len, 8);
	uint32_t tag_len = sizeof(ORE_MERGE);
	// mergeTag + nonce + row
	return AES_BLOCK_LEN + (cmp_len* nslots + tag_len) * nblocks;
}

int ore_query_init(ore_query ctxt, ore_params params)
{
	if (ctxt == NULL || params == NULL) {
		return ERROR_NULL_POINTER;
	}

	if (!ore_is_valid_params(params)) {
		return ERROR_PARAMS_INVALID;
	}

	ctxt->comp_left = malloc(ore_query_left_len(params));

	if (ctxt->comp_left == NULL) {
		return ERROR_MEMORY_ALLOCATION;
	}

	ctxt->initialized = true;

	return ERROR_NONE;
}

int ore_query_cleanup(ore_query ctxt, ore_params params)
{
	if (ctxt == NULL)
	{
		return ERROR_NONE;
	}
	if (!ore_is_valid_params(params))
	{
		return ERROR_PARAMS_INVALID;
	}

	free(ctxt->comp_left);

	return ERROR_NONE;
}

int ore_index_init(ore_index ctxt, ore_params params)
{
	if (ctxt == NULL || params == NULL) {
		return ERROR_NULL_POINTER;
	}

	if (!ore_is_valid_params(params)) {
		return ERROR_PARAMS_INVALID;
	}

	uint32_t right_len = ore_index_right_len(params);

	ctxt->comp_right = malloc(right_len);
	if (ctxt->comp_right == NULL) {
		return ERROR_MEMORY_ALLOCATION;
	}

	ctxt->initialized = true;

	return ERROR_NONE;
}

int ore_index_cleanup(ore_index ctxt, ore_params params)
{
	if (ctxt == NULL)
	{
		return ERROR_NONE;
	}
	if (!ore_is_valid_params(params))
	{
		return ERROR_PARAMS_INVALID;
	}
	uint32_t right_len = ore_index_right_len(params);
	free(ctxt->comp_right);

	return ERROR_NONE;
}

int ore_compare(int* result_p, ore_query query1, ore_query query2, ore_index ctxt2, ore_params params)
{
	*result_p = -1;

	if (!query1->initialized || !query2->initialized || !ctxt2->initialized) {
		return ERROR_CTXT_NOT_INITIALIZED;
	}

	if (!ore_is_valid_params(params)) {
		return ERROR_PARAMS_INVALID;
	}

	uint32_t nbits = params->nbits;
	uint32_t block_len = params->block_len;
	uint32_t cmp_len = CEIL(params->cmp_len, 8);
	uint32_t nslots = 1 << block_len;
	uint32_t nblocks = CEIL(nbits, block_len);

	block nonce = *(block*)ctxt2->comp_right;

	//uint64_t* block_loc = malloc(sizeof(uint64_t) * (1 << 8));
	//ORE_AES_KEY loc_key;
	//ERR_CHECK(setup_aes_key(&loc_key, (byte*)&nonce, sizeof(block)));
	//block_prp_loc(block_loc, loc_key, nblocks, true);
	//if (DEBUG_INFO)
	//{
	//	printf("nblocks:%d\n", nblocks);
	//	printf("blkLoc in cmp: ");
	//	for (int i = 0; i < nblocks; i++)
	//	{
	//		printf("%d ", block_loc[i]);
	//	}
	//	printf("\n");
	//}

	uint32_t offset_left = 0;
	uint32_t offset_right = sizeof(block);

	uint32_t len_left_block = AES_BLOCK_LEN;
	uint32_t len_right_block = nslots * (cmp_len) * 2 +sizeof(ORE_MERGE);

	ORE_AES_KEY cmp_key_nonce;
	ERR_CHECK(setup_aes_key(&cmp_key_nonce, (byte*)&nonce, AES_KEY_BYTES));

	bool minMatch = false;
	bool maxMatch = false;

	//cmp merge token and min token
	for (int i = 0; i < nblocks; i++)
	{

		block key_block, tropdoor_block;
		memcpy(&key_block, query1->comp_left + offset_left, AES_KEY_BYTES);
		ERR_CHECK(aes_eval(&tropdoor_block, &cmp_key_nonce, key_block));

		uint64_t num = 0;
		memcpy(&num, &tropdoor_block, cmp_len);

		uint64_t right;

		int mergeTag;
		memcpy(&mergeTag, ctxt2->comp_right + offset_right, sizeof(mergeTag));

		if (DEBUG_INFO)
		{
			printf("cmp1 at block %d-->%d, mergeTag %d.\n", i, i, mergeTag);
		}
		if (mergeTag == ORE_MERGE)
		{
			for (int k = 0; k < nslots; k++)
			{
				memcpy(&right, ctxt2->comp_right + offset_right + sizeof(mergeTag) + k*cmp_len, cmp_len);
				if (num == right)
				{
					minMatch = true;
					maxMatch = true;
					if (DEBUG_INFO)
					{
						printf("at block %d:%d, match %d.\n", i, k, 0);
					}
					break;
				}
				else if (num + 1 == right)
				{
					minMatch = true;
					if (DEBUG_INFO)
					{
						printf("at block %d:%d, match %d.\n", i, k, 1);
					}
					break;
				}
			}
			if (minMatch)
			{
				break;
			}
		}
		else
		{
			for (int k = 0; k < nslots; k++)
			{
				memcpy(&right, ctxt2->comp_right + offset_right + sizeof(mergeTag) + k*cmp_len, cmp_len);
				if (num + 1 == right)
				{
					minMatch = true;
					if (DEBUG_INFO)
					{
						printf("at block %d:%d, match %d.\n", i, k, 0);
					}
					break;
				}
			}

			if (minMatch)
			{
				break;
			}
		}

		offset_left += len_left_block;
		offset_right += len_right_block;
	}

	//cmp max token
	if (!maxMatch)
	{
		offset_left = 0;
		offset_right = sizeof(block);
		for (int i = 0; i < nblocks; i++)
		{

			block key_block2, tropdoor_block2;
			memcpy(&key_block2, query2->comp_left + offset_left, AES_KEY_BYTES);
			ERR_CHECK(aes_eval(&tropdoor_block2, &cmp_key_nonce, key_block2));

			uint64_t num2 = 0;
			memcpy(&num2, &tropdoor_block2, cmp_len);

			uint64_t right;

			int mergeTag;
			memcpy(&mergeTag, ctxt2->comp_right + offset_right, sizeof(mergeTag));

			if (DEBUG_INFO)
			{
				printf("cmp2 at block %d-->%d, mergeTag %d.\n", i, i, mergeTag);
			}
			if (mergeTag == ORE_MERGE)
			{
				for (int k = 0; k < nslots; k++)
				{
					memcpy(&right, ctxt2->comp_right + offset_right + sizeof(mergeTag) + k*cmp_len, cmp_len);
					if (num2 + 2 == right)
					{
						maxMatch = true;
						if (DEBUG_INFO)
						{
							printf("at block %d:%d, match %d.\n", i, k, 2);
						}
						break;
					}
				}

				if (maxMatch)
				{
					break;
				}
			}
			else
			{
				for (int k = 0; k < nslots; k++)
				{
					memcpy(&right, ctxt2->comp_right + offset_right + sizeof(mergeTag) + nslots * cmp_len + k*cmp_len, cmp_len);
					if (num2 + 2 == right)
					{
						maxMatch = true;
						if (DEBUG_INFO)
						{
							printf("at block %d:%d, match %d.\n", i, k, 2);
						}
						break;
					}
				}
				if (maxMatch)
				{
					break;
				}
			}

			offset_left += len_left_block;
			offset_right += len_right_block;
		}
	}

	//free(block_loc);

	if (minMatch&&maxMatch)
	{
		*result_p = 1;
	}

	return ERROR_NONE;
}
