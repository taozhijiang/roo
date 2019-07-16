/*-
 * Copyright (c) 2019 TAO Zhijiang<taozhijiang@gmail.com>
 *
 * Licensed under the BSD-3-Clause license, see LICENSE for full information.
 *
 */


#ifndef __ROO_CONNECT_SQL_CONN_H__
#define __ROO_CONNECT_SQL_CONN_H__

#include <vector>

#if __cplusplus >= 201103L
#include <type_traits>
#else
#include <boost/type_traits.hpp>
#endif

#include <string>


#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <connect/ConnPool.h>


namespace roo {
    
#if 0

// 接口汇总
template<typename T>
bool cast_value(shared_result_ptr result, const uint32_t idx, T& val);
template<typename T, typename ... Args>
bool cast_value(shared_result_ptr result, const uint32_t idx, T& val, Args& ... rest);

inline int rs_is_null(shared_result_ptr result, const uint32_t idx);


// Class SqlConn
bool execute(const std::string& sql);
sql::ResultSet* execute_select(const std::string& sql);
int  execute_update(const std::string& sql);

// 只返回一条记录的时候，返回true
template<typename T>
bool select_one(const std::string& sql, T& val);
template<typename ... Args>
bool select_one(const std::string& sql, Args& ... rest);

// 执行有错误返回false，否则返回true(真正的vector可能是空的)
template<typename T>
bool select_multi(const std::string& sql, std::vector<T>& vec);

// 如果有多条记录，多个列，使用execute_select手动进行查询

#endif 
    

class SqlConn;
typedef std::shared_ptr<SqlConn> sql_conn_ptr;
typedef std::unique_ptr<sql::ResultSet> scoped_result_ptr;
typedef std::shared_ptr<sql::ResultSet> shared_result_ptr;

struct SqlConnPoolHelper {
public:
    SqlConnPoolHelper(std::string host, int port,
                      std::string user, std::string passwd, std::string db) :
        host_(host), port_(port),
        user_(user), passwd_(passwd), db_(db) {
    }

public:
    const std::string host_;
    const int port_;
    const std::string user_;
    const std::string passwd_;
    const std::string db_;
    const std::string charset_;
};

template<typename T>
bool cast_value(shared_result_ptr result, const uint32_t idx, T& val) {

    try {

#if __cplusplus >= 201103L

        if (std::is_floating_point<T>::value) {
            val = static_cast<T>(result->getDouble(idx));
        }
        else if (std::is_integral<T>::value) {
            if (std::is_signed<T>::value) {
                val = static_cast<T>(result->getInt64(idx));
            } else {
                val = static_cast<T>(result->getUInt64(idx));
            }
        }

#else

        if (boost::is_floating_point<T>::value) {
            val = static_cast<T>(result->getDouble(idx));
        }
        else if (boost::is_integral<T>::value) {
            if (boost::is_signed<T>::value) {
                val = static_cast<T>(result->getInt64(idx));
            } else {
                val = static_cast<T>(result->getUInt64(idx));
            }
        }

#endif
        else {
            log_err("Tell unsupported type: %s", typeid(T).name());
            return false;
        }

    } catch (sql::SQLException& e) {

        std::stringstream output;
        output << "# ERR: " << e.what() << std::endl;
        output << " (MySQL error code: " << e.getErrorCode() << std::endl;
        output << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        log_err("%s", output.str().c_str());

        return false;
    }

    return true;
}

// 特例化如果多次包含连接会重复定义，所以要么static、inline
template<>
inline bool cast_value(shared_result_ptr result, const uint32_t idx, std::string& val) {

    try {
        val = static_cast<std::string>(result->getString(static_cast<int32_t>(idx)));
    } catch (sql::SQLException& e) {

        std::stringstream output;
        output << "# ERR: " << e.what() << std::endl;
        output << " (MySQL error code: " << e.getErrorCode() << std::endl;
        output << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        log_err("%s", output.str().c_str());

        return false;
    }

    return true;
}

// 可变模板参数进行查询
template<typename T, typename ... Args>
bool cast_value(shared_result_ptr result, const uint32_t idx, T& val, Args& ... rest) {

    cast_value(result, idx, val);
    return cast_value(result, idx+1, rest ...);
}

// -1 err, 0 false, 1 true
// Column index, first column is 1, second is 2,...
inline int rs_is_null(shared_result_ptr result, const uint32_t idx) {

    try {
        if (!result) return -1;
        return result->isNull(idx) ? 1 : 0;
    } catch (sql::SQLException& e) {

        std::stringstream output;
        output << "# ERR: " << e.what() << std::endl;
        output << " (MySQL error code: " << e.getErrorCode() << std::endl;
        output << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        log_err("%s", output.str().c_str());
    }

    return -1;
}

class SqlConn : public ConnStat {
public:
    explicit SqlConn(ConnPool<SqlConn, SqlConnPoolHelper>& pool, const SqlConnPoolHelper& helper);
    virtual ~SqlConn();

    // 禁止拷贝
    SqlConn(const SqlConn&) = delete;
    SqlConn& operator=(const SqlConn&) = delete;

    bool init(int64_t conn_uuid);
    bool ping_test();

    bool is_health() {
        return (conn_ && conn_->isValid());
    }

    // Simple SQL API
    bool execute(const std::string& sql);
    sql::ResultSet* execute_select(const std::string& sql);
    int  execute_update(const std::string& sql);

    // 常用操作，当取到的记录恰好是1条的时候返回true
    template<typename T>
    bool select_one(const std::string& sql, T& val);
    template<typename ... Args>
    bool select_one(const std::string& sql, Args& ... rest);

    // 执行有错误返回false，否则返回true(真正的vector可能是空的)
    template<typename T>
    bool select_multi(const std::string& sql, std::vector<T>& vec);

    bool begin_transaction() { return execute("START TRANSACTION"); }
    bool commit() { return execute("COMMIT"); }
    bool rollback() { return execute("ROLLBACK"); }

private:
    sql::Driver* driver_;   /* no need explicit free */

    std::unique_ptr<sql::Connection> conn_;
    std::unique_ptr<sql::Statement> stmt_;

    // may be used in future
    ConnPool<SqlConn, SqlConnPoolHelper>& pool_;
    const SqlConnPoolHelper helper_;
    int64_t conn_uuid_;
};


template<typename T>
bool SqlConn::select_one(const std::string& sql, T& val) {
    try {

        if (!conn_->isValid()) {
            log_err("Invalid connect, do re-connect...");
            conn_->reconnect();
        }

        stmt_->execute(sql);
        shared_result_ptr result(stmt_->getResultSet());
        if (!result)
            return false;

        if (result->rowsCount() != 1) {
            log_err("Error rows count: %lu", result->rowsCount());
            return false;
        }

        if (result->next())
            return cast_value(result, 1, val);

        return false;

    } catch (sql::SQLException& e) {

        std::stringstream output;
        output << " STMT: " << sql << std::endl;
        output << "# ERR: " << e.what() << std::endl;
        output << " (MySQL error code: " << e.getErrorCode() << std::endl;
        output << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        log_err("%s", output.str().c_str());

        return false;
    }
}



template<typename ... Args>
bool SqlConn::select_one(const std::string& sql, Args& ... rest) {

    try {
        if (!conn_->isValid()) {
            log_err("Invalid connect, do re-connect...");
            conn_->reconnect();
        }

        stmt_->execute(sql);
        shared_result_ptr result(stmt_->getResultSet());
        if (!result)
            return false;

        if (result->rowsCount() != 1) {
            log_err("Error rows count: %lu", result->rowsCount());
            return false;
        }

        if (result->next())
            return cast_value(result, 1, rest ...);

        return false;

    } catch (sql::SQLException &e){

        std::stringstream output;
        output << " STMT: " << sql << std::endl;
        output << "# ERR: " << e.what() << std::endl;
        output << " (MySQL error code: " << e.getErrorCode() << std::endl;
        output << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        log_err("%s", output.str().c_str());

        return false;
    }
}



template<typename T>
bool SqlConn::select_multi(const std::string& sql, std::vector<T>& vec) {

    try {

        if (!conn_->isValid()) {
            log_err("Invalid connect, do re-connect...");
            conn_->reconnect();
        }

        stmt_->execute(sql);
        shared_result_ptr result(stmt_->getResultSet());
        if (!result)
            return false;

        vec.clear();
        T r_val;
        while (result->next()) {
            if (cast_value(result, 1, r_val)) 
                vec.push_back(r_val);
            else 
                return false;
        }

        return true;

    } catch (sql::SQLException& e) {

        std::stringstream output;
        output << " STMT: " << sql << std::endl;
        output << "# ERR: " << e.what() << std::endl;
        output << " (MySQL error code: " << e.getErrorCode() << std::endl;
        output << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        log_err("%s", output.str().c_str());

        return false;
    }
}

} // end namespace roo

#endif  // __ROO_CONNECT_SQL_CONN_H__
