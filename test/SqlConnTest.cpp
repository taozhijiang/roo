#include <gmock/gmock.h>
#include <string>

#include <iostream>

#include <connect/SqlConn.h>

using namespace ::testing;
using namespace roo;

class SqlConnSt : public ::testing::Test {

protected:
    void SetUp() {
        ASSERT_THAT(sql_pool_.init(), Eq(true));
    }

    void TearDown() {
    }

public:

    SqlConnSt() :
        sql_pool_("MySQLPool", 5,
                  SqlConnPoolHelper("localhost", 3306,
                                    "root", "1234", "test")) {
    }

    virtual ~SqlConnSt() {
    }

    ConnPool<SqlConn, SqlConnPoolHelper> sql_pool_;
};


TEST_F(SqlConnSt, PoolTest) {

    ASSERT_THAT(sql_pool_.get_capacity(), Eq(5));

    sql_conn_ptr conn, conn2;
    sql_pool_.request_scoped_conn(conn);
    ASSERT_THAT(!!conn, Eq(true));
    ASSERT_THAT(sql_pool_.get_busy_size(), Eq(1));

    sql_pool_.request_scoped_conn(conn2);
    ASSERT_THAT(!!conn2, Eq(true));

    conn.reset();
    ASSERT_THAT(sql_pool_.get_idle_size() == 1 && sql_pool_.get_busy_size() == 1, Eq(true));
}



TEST_F(SqlConnSt, SqlConnDataTypeTest) {

    sql_conn_ptr conn;
    sql_pool_.request_scoped_conn(conn);
    ASSERT_THAT(!!conn, Eq(true));

    std::string sql = "SELECT 123, 'ttz', 10.5;";
    shared_result_ptr result;
    result.reset(conn->execute_select(sql));
    ASSERT_THAT(result && result->next(), Eq(true));

    int32_t     int_val{};
    std::string str_val{};
    double      flt_val{};

    bool success = cast_value(result, 1, int_val, str_val, flt_val);

    ASSERT_THAT(success && int_val == 123 && str_val == "ttz", Eq(true));
    ASSERT_THAT(flt_val, DoubleEq(10.5));
}

