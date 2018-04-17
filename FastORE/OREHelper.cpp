#include "OREHelper.h"
#include <assert.h>
#include <unistd.h>


OREHelper::OREHelper()
{
}


OREHelper::~OREHelper()
{
	if (m_key->initialized)
	{
		ore_key_cleanup(m_key);
	}

}


void OREHelper::Init(string mainKey, uint32_t uiBlockSize)
{
	
	if (initOnce)
	{
		sleep(1);
		return;
	}
	initOnce = true;
	init_ore_params(m_params, 32, uiBlockSize, 64);
	ore_key_setup(const_cast<char*>(mainKey.c_str()), m_key, m_params);
}

//p is the point to memory that has uiLen bytes
//iCmp is the compare enum, >0 means to > and <0 means <
//szTable is the table name
//szCol is the column name
//uiVal is the value
//RETURN is the actual size of LeftPart ORE
uint32_t OREHelper::CreateLeft(char *p, uint32_t uiLen, int iCmp, uint32_t uiVal)
{
	uint32_t len_left = ore_query_left_len(m_params);

	assert(uiLen >= len_left);
	ore_query m_query;
	ore_query_init(m_query, m_params);

	int m_cmp = iCmp > 0 ? ORE_LARGE : ORE_SMALL;

	ore_query_setup(m_query, m_key, uiVal,  m_cmp);

	memcpy(p, m_query->comp_left, len_left);

	ore_query_cleanup(m_query, m_params);

	return len_left;
}


//p is the point to memory that has uiLen bytes
//szTable is the table name
//szCol is the column name
//uiVal is the value
//iCounter is the counter c
//RETURN is the actual size of LeftPart ORE
uint32_t OREHelper::CreateRight(char *p, uint32_t uiLen, uint32_t uiValMin, uint32_t uiValMax)
{
	uint32_t len_right = ore_index_right_len(m_params);
	assert(uiLen >= len_right);

	ore_index m_index;
	ore_index_init(m_index, m_params);
	ore_index_setup(m_index, m_key, uiValMin, uiValMax);

	memcpy(p, m_index->comp_right, len_right);

	ore_index_cleanup(m_index, m_params);

	return len_right;
}


//pLeft is the point to ORE_Left Part which is uiLeft bytes
//pRight is the point to ORE_Right part which is uiRight Bytes
//iCounter is the counter c
//RETURN is the enum which show the result : 1 means YES, -1 means NO 
uint32_t OREHelper::CompareORE(char *pLeftMin, uint32_t uiLeftMin, char* pLeftMax, uint32_t uiLeftMax, char *pRight, uint32_t uiRight)
{
	uint32_t len_left = ore_query_left_len(m_params);
	assert(uiLeftMin >= len_left);
	assert(uiLeftMax >= len_left);
	uint32_t len_right = ore_index_right_len(m_params);
	assert(uiRight >= len_right);

	ore_query m_query;
	ore_query_init(m_query, m_params);

	ore_query m_query2;
	ore_query_init(m_query2, m_params);

	ore_index m_index;
	ore_index_init(m_index, m_params);

	memcpy(m_query->comp_left, pLeftMin, len_left);
	memcpy(m_query2->comp_left, pLeftMax, len_left);
	memcpy(m_index->comp_right, pRight, len_right);

	int res;

	ore_compare(&res, m_query, m_query2, m_index, m_params);

	ore_query_cleanup(m_query, m_params);
	ore_index_cleanup(m_index, m_params);

	return res;
}

