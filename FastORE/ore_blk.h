#ifndef __ORE_BLK_H__
#define __ORE_BLK_H__


#include <stdbool.h>
#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ORE_SMALL (1)
#define ORE_EQUAL (2)
#define ORE_LARGE (3)

#define ORE_ZERO (0)
#define ORE_ONE (1)
#define ORE_TWO (2)
#define ORE_PADDING (3)

#define ORE_MERGE (1)
#define ORE_NOT_MERGE (2)

	typedef struct {
		bool initialized;    // whether or not these parameters have been initialized
		uint32_t nbits;      // the number of bits in the plaintext elements,max is 64
		uint32_t block_len;  // the number of bits in each block of the plaintext, max is 16
		uint32_t cmp_len;    // the number of bits in compare result,max is 64
	} ore_params[1];
	typedef struct {
		bool initialized;         // whether or not the secret key has been initalized
		char			org_key[128];  // the original key from user input
		ORE_AES_KEY         prf_key;  // key for the PRF (for deriving the keys from each prefix)
		ORE_AES_KEY         prp_key;  // key for the PRP (for permuting the slots within a block)
		ORE_AES_KEY         prg_key;  // key for the PRG (for permuting the slots within a block)
		uint64_t		prg_counter;// current prg msg
		ore_params  params;
	} ore_key[1];
	typedef struct {
		bool initialized;
		byte*           comp_left;   // the left ciphertext
	} ore_query[1];
	typedef struct {
		bool initialized;
		byte*           comp_right;  // the right ciphertext
	} ore_index[1];

   /**
	* Initialize the ore function parameters like init_ore_blk_params 
	*
	* @param params     The params to initialize
	* @param nbits      The number of bits of an input to the encryption scheme
	* @param block_len  The length (in bits) of each block in the plaintext space
	* @param cmp_len    The compare result length we used in ore_right.
	*
	* @return ERROR_NONE on success and a corresponding error code on failure
	*         (see errors.h for the full list of possible error codes)
	*/
	int init_ore_params(ore_params params, uint32_t nbits, uint32_t block_len, uint32_t cmp_len);
    /*	
	 * A group of function to control ore_key
	 */
	int ore_key_setup(char* input, ore_key& sk, ore_params params);
	int ore_key_cleanup(ore_key& sk);

	/*
	* A group of function to control ore_query
	*/
	int ore_query_left_len(ore_params params);
	int ore_query_init(ore_query ctxt, ore_params params);
	int ore_query_setup(ore_query ctxt, ore_key& sk, uint64_t msg, uint8_t aim);
	int ore_query_cleanup(ore_query ctxt, ore_params params);

	/*
	* A group of function to control ore_index
	*/
	int ore_index_right_len(ore_params params);
	int ore_index_init(ore_index ctxt, ore_params params);
	int ore_index_setup(ore_index ctxt, ore_key& sk, uint64_t msg1, uint64_t msg2);
	int ore_index_cleanup(ore_index ctxt, ore_params params);

	/**
	* Performs the comparison of ore_query and ore_index to determine the ordering whether
	* correspond the query need.
	*
	* @param result_p A pointer containing the result of the comparison, which is 1
	*                 if compare success, -1 if compare failed, and 0 if they encrypt
	*                 equal plaintexts.
	* @param ctxt1    The compare query include the comparison.
	* @param ctxt2    The index to be compared.
	* @param params   The ore_parames will be used in this function.
	* @param rnum     The input row No.
	*
	* @return ERROR_NONE on success, and a corresponding error code on failure
	*         (see errors.h for the full list of possible error codes)
	*/
	int ore_compare(int* result_p, ore_query ctxt1, ore_query ctxt2, ore_index ctxt3, ore_params params);

#ifdef __cplusplus
}
#endif

#endif