//
//  mysql_database.h
//  thread
//
//  Created by 冯文斌 on 16/12/30.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef mysql_database_h
#define mysql_database_h
#include <mysql.h>

enum KE_DBCREATE_FLAG
{
    emKDBCREATE_IF_NOT_EXIST = 1,
};
enum KE_TABLE_CREATE_FLAG
{
    emKTABLE_CREATE_IF_NOT_EXIST = 1,
};
enum KE_GETRESULT_METHOD
{
    emKGET_RESULT_STORE = 1,
};

const unsigned KD_TINY_FIELD_MAX_DATA_SIZE = 255;

// max size of type KGD_TYPE_TEXT, KGD_TYPE_BLOB
const unsigned KD_MEDIUM_FIELD_MAX_DATA_SIZE = 65535;

const unsigned KD_BLOB_OR_TEXT_INDEX_PREFIX_MAX_NUM = 255;

const unsigned KD_MEDIUM_BLOB_MAX_SIZE = 16777215;

#define KD_MYSQL_GLOBAL_TABLE_NAME "global_variable"

/***************************** Use for getting query result *******************************/
struct KMySQLFields
{
public:
    KMySQLFields(MYSQL_FIELD *pMySQLField) : aFieldSt(pMySQLField) {}
    MYSQL_FIELD *aFieldSt;
};

struct KMySQLRow
{
public:
    KMySQLRow(MYSQL_RES *pMySQLRes)
    {
        //m_nFieldCount = mysql_num_fields(pMySQLRes);
        aFieldData = mysql_fetch_row(pMySQLRes);
        aFieldLenth = mysql_fetch_lengths(pMySQLRes);
    }
    bool IsValid() { return aFieldData != NULL; }
    MYSQL_ROW	aFieldData;
    unsigned long* aFieldLenth;
};

class KMySQLResult
{
public:
    KMySQLResult() { m_pMySQLRes = NULL; }
    KMySQLResult(MYSQL_RES *pMySQLRes) : m_pMySQLRes(pMySQLRes) {}
    int GetRowCount() { return (int)mysql_num_rows(m_pMySQLRes); }
    int GetFieldCount() { return mysql_num_fields(m_pMySQLRes); }
    KMySQLFields GetFieldsDesc() { return mysql_fetch_fields(m_pMySQLRes); }
    KMySQLRow GetNextRow() { return KMySQLRow(m_pMySQLRes); }
    void SeekData(unsigned long lPos) { mysql_data_seek(m_pMySQLRes, lPos); }
    operator MYSQL_RES *() { return m_pMySQLRes; }
    bool IsValid() { return m_pMySQLRes != NULL; }
    void Release() { if (m_pMySQLRes) mysql_free_result(m_pMySQLRes); m_pMySQLRes = NULL; }
protected:
    MYSQL_RES *m_pMySQLRes;
};

class KMySQLResult_Protected : public KMySQLResult
{
public:
    KMySQLResult_Protected(const KMySQLResult& rs) : KMySQLResult(rs) {}
    ~KMySQLResult_Protected()
    {
        Release();
    }
};

#endif /* mysql_database_h */
