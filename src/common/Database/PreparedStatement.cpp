/*
* Copyright (C) 2016+     AzerothCore <www.azerothcore.org>
* Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
*/

#include "PreparedStatement.h"
#include "Errors.h"
#include "MySQLConnection.h"
#include "MySQLPreparedStatement.h"
#include "QueryResult.h"
#include "Log.h"
#include "MySQLWorkaround.h"

PreparedStatement::PreparedStatement(uint32 index, uint8 capacity) :
m_stmt(nullptr), m_index(index), statement_data(capacity) { }

PreparedStatement::~PreparedStatement() { }

void PreparedStatement::BindParameters(MySQLPreparedStatement* stmt)
{
    ASSERT(stmt);
    m_stmt = stmt;

    uint8 i = 0;
    for (; i < statement_data.size(); i++)
    {
        switch (statement_data[i].type)
        {
            case TYPE_BOOL:
                stmt->setBool(i, statement_data[i].data.boolean);
                break;
            case TYPE_UI8:
                stmt->setUInt8(i, statement_data[i].data.ui8);
                break;
            case TYPE_UI16:
                stmt->setUInt16(i, statement_data[i].data.ui16);
                break;
            case TYPE_UI32:
                stmt->setUInt32(i, statement_data[i].data.ui32);
                break;
            case TYPE_I8:
                stmt->setInt8(i, statement_data[i].data.i8);
                break;
            case TYPE_I16:
                stmt->setInt16(i, statement_data[i].data.i16);
                break;
            case TYPE_I32:
                stmt->setInt32(i, statement_data[i].data.i32);
                break;
            case TYPE_UI64:
                stmt->setUInt64(i, statement_data[i].data.ui64);
                break;
            case TYPE_I64:
                stmt->setInt64(i, statement_data[i].data.i64);
                break;
            case TYPE_FLOAT:
                stmt->setFloat(i, statement_data[i].data.f);
                break;
            case TYPE_DOUBLE:
                stmt->setDouble(i, statement_data[i].data.d);
                break;
            case TYPE_STRING:
                stmt->setBinary(i, statement_data[i].binary, true);
                break;
            case TYPE_BINARY:
                stmt->setBinary(i, statement_data[i].binary, false);
                break;
            case TYPE_NULL:
                stmt->setNull(i);
                break;
        }
    }
    #ifdef _DEBUG
    if (i < stmt->m_paramCount)
        sLog->outSQLDriver("[WARNING]: BindParameters() for statement %u did not bind all allocated parameters", m_index);
    #endif
}

//- Bind to buffer
void PreparedStatement::setBool(const uint8 index, const bool value)
{
    ASSERT(index < statement_data.size());
    statement_data[index].data.boolean = value;
    statement_data[index].type = TYPE_BOOL;
}

void PreparedStatement::setUInt8(const uint8 index, const uint8 value)
{
    ASSERT(index < statement_data.size());
    statement_data[index].data.ui8 = value;
    statement_data[index].type = TYPE_UI8;
}

void PreparedStatement::setUInt16(const uint8 index, const uint16 value)
{
    ASSERT(index < statement_data.size());
    statement_data[index].data.ui16 = value;
    statement_data[index].type = TYPE_UI16;
}

void PreparedStatement::setUInt32(const uint8 index, const uint32 value)
{
    ASSERT(index < statement_data.size());
    statement_data[index].data.ui32 = value;
    statement_data[index].type = TYPE_UI32;
}

void PreparedStatement::setUInt64(const uint8 index, const uint64 value)
{
    ASSERT(index < statement_data.size());
    statement_data[index].data.ui64 = value;
    statement_data[index].type = TYPE_UI64;
}

void PreparedStatement::setInt8(const uint8 index, const int8 value)
{
    ASSERT(index < statement_data.size());
    statement_data[index].data.i8 = value;
    statement_data[index].type = TYPE_I8;
}

void PreparedStatement::setInt16(const uint8 index, const int16 value)
{
    ASSERT(index < statement_data.size());
    statement_data[index].data.i16 = value;
    statement_data[index].type = TYPE_I16;
}

void PreparedStatement::setInt32(const uint8 index, const int32 value)
{
    ASSERT(index < statement_data.size());
    statement_data[index].data.i32 = value;
    statement_data[index].type = TYPE_I32;
}

void PreparedStatement::setInt64(const uint8 index, const int64 value)
{
    ASSERT(index < statement_data.size());
    statement_data[index].data.i64 = value;
    statement_data[index].type = TYPE_I64;
}

void PreparedStatement::setFloat(const uint8 index, const float value)
{
    ASSERT(index < statement_data.size());
    statement_data[index].data.f = value;
    statement_data[index].type = TYPE_FLOAT;
}

void PreparedStatement::setDouble(const uint8 index, const double value)
{
    ASSERT(index < statement_data.size());
    statement_data[index].data.d = value;
    statement_data[index].type = TYPE_DOUBLE;
}

void PreparedStatement::setString(const uint8 index, const std::string& value)
{
    ASSERT(index < statement_data.size());
    statement_data[index].binary.resize(value.length() + 1);
    memcpy(statement_data[index].binary.data(), value.c_str(), value.length() + 1);
    statement_data[index].type = TYPE_STRING;
}

void PreparedStatement::setBinary(const uint8 index, const std::vector<uint8>& value)
{
    ASSERT(index < statement_data.size());
    statement_data[index].binary = value;
    statement_data[index].type = TYPE_BINARY;
}

void PreparedStatement::setNull(const uint8 index)
{
    ASSERT(index < statement_data.size());
    statement_data[index].type = TYPE_NULL;
}

//- Execution
PreparedStatementTask::PreparedStatementTask(PreparedStatement* stmt, bool async) :
m_stmt(stmt), m_result(nullptr)
{
    m_has_result = async; // If it's async, then there's a result
    if (async)
        m_result = new PreparedQueryResultPromise();
}

PreparedStatementTask::~PreparedStatementTask()
{
    delete m_stmt;
    if (m_has_result && m_result != nullptr)
        delete m_result;
}

bool PreparedStatementTask::Execute()
{
    if (m_has_result)
    {
        PreparedResultSet* result = m_conn->Query(m_stmt);
        if (!result || !result->GetRowCount())
        {
            delete result;
            m_result->set_value(PreparedQueryResult(nullptr));
            return false;
        }
        m_result->set_value(PreparedQueryResult(result));
        return true;
    }

    return m_conn->Execute(m_stmt);
}
