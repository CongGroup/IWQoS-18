#ifndef __ORE_HELPER_H__
#define __ORE_HELPER_H__

#include <string.h>
#include <iostream>
#include <stdint.h>

#include "ore_blk.h"

using namespace std;

#define DEF_PRF_OUTPUT_SIZE 16
//Plaintext size in bit
#define DEF_PLAINTEXT_SIZE_INBIT 32

//Block size in bit
#define DEF_BLOCK_SIZE_INBIT 8

//Block minimal size in bit
#define DEF_BLOCK_MINSIZE_INBIT 2

//Compare flag (in Ciphertext ORE_RIGHT) size in bit
#define DEF_COMPARE_SIZE_INBIT 64

//The MAX size of whole ORE_Left in byte
#define DEF_ORELEFT_MAXSIZE ((DEF_PRF_OUTPUT_SIZE ) * DEF_PLAINTEXT_SIZE_INBIT / DEF_BLOCK_MINSIZE_INBIT)

//The MAX size of whole ORE_RIGHT in byte
#define DEF_ORERIGHT_MAXSIZE (DEF_PRF_OUTPUT_SIZE + (sizeof(int) + (DEF_COMPARE_SIZE_INBIT / 8 ) * ( 1 << DEF_BLOCK_SIZE_INBIT )) * DEF_PLAINTEXT_SIZE_INBIT * 2 / DEF_BLOCK_MINSIZE_INBIT)



class OREHelper
{
public:
	OREHelper();
	~OREHelper();

	void Init(string mainKey, uint32_t uiBlockSize);

	/**
	* Create the query data
	*
	* @param p is the point to memory that has uiLen bytes
	* @param iCmp is the compare enum, >0 means to > and <0 means <
	* @param uiVal is the value
	*
	* @return return the actual size of LeftPart ORE
	*/
	uint32_t CreateLeft(char *p, uint32_t uiLen, int iCmp,  uint32_t uiVal);

	/**
	* Create the index data
	*
	* @param p is the point to memory that has uiLen bytes
	* @param uiVal is the value
	*
	* @return return is the actual size of LeftPart ORE
	*/
	uint32_t CreateRight(char *p, uint32_t uiLen, uint32_t uiValMin, uint32_t uiValMax);

	/**
	* Create the index data
	*
	* @param pLeft is the point to ORE_Left Part which is uiLeft bytes
	* @param pRight is the point to ORE_Right part which is uiRight Bytes
	*
	* @return return is the enum which show the result : 0 means equal, 1 means YES, others means NO 
	*/

	uint32_t CompareORE(char *pLeftMin, uint32_t uiLeftMin, char* pLeftMax, uint32_t uiLeftMax, char *pRight, uint32_t uiRight);

protected:
	string currentKey;
	uint32_t currentBlockSize;

	ore_params m_params;
	ore_key m_key;

	bool initOnce = false;
	//ore_query m_query;
	//ore_index m_index;

};

#endif
