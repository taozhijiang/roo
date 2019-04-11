#include <gmock/gmock.h>
#include <string>

#include <iostream>

#include <connect/SqlConn.h>

using namespace ::testing;
using namespace roo;

TEST(SqlConnTest, SqlConnDataTypeTest) {

    SqlConnPoolHelper helper("localhost", 3306,
                             "root", "1234", "test");

    std::shared_ptr<ConnPool<SqlConn, SqlConnPoolHelper>> sql_pool_ptr_;
    sql_pool_ptr_.reset(new ConnPool<SqlConn, SqlConnPoolHelper>("MySQLPool", 5, helper));
    ASSERT_THAT (sql_pool_ptr_  && sql_pool_ptr_->init(), true);


    sql_conn_ptr conn;
    sql_pool_ptr_->request_scoped_conn(conn);
    ASSERT_THAT(!!conn, Eq(true));

    std::string sql = "SELECT 123, 'ttz', 10.5;";
    shared_result_ptr result;
    result.reset(conn->sqlconn_execute_query(sql));
    ASSERT_THAT (result && result->next(), Eq(true));

    int32_t     int_val {};
    std::string str_val {};
    double      flt_val {};

    bool success = cast_raw_value(result, 1, int_val, str_val, flt_val);

    ASSERT_THAT(success && int_val == 123 && str_val == "ttz", Eq(true));
    ASSERT_THAT(flt_val, DoubleEq(10.5));

}

