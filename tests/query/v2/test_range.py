# --coding:utf-8--
#
# Copyright (c) 2020 vesoft inc. All rights reserved.
#
# This source code is licensed under Apache 2.0 License,
# attached with Common Clause Condition 1.0, found in the LICENSES directory.

import time

import pytest
from nebula2.graph import ttypes

from tests.common.nebula_test_suite import NebulaTestSuite


class TestYield(NebulaTestSuite):
    @classmethod
    def prepare(self):
        self.use_nba()

    def test_range(self):
        query = 'YIELD range(1,5)'
        resp = self.execute_query(query)
        self.check_resp_succeeded(resp)
        self.check_column_names(resp, ["range(1,5)"])
        self.check_result(resp, [[[1,2,3,4]]])

        query = "YIELD range(1,5,2)"
        resp = self.execute_query(query)
        self.check_resp_succeeded(resp)
        columns = ["range(1,5,2)"]
        self.check_column_names(resp, columns)
        expect_result = [[[1,3]]]
        self.check_result(resp, expect_result)

        query = 'YIELD range(5,1,2)'
        resp = self.execute_query(query)
        self.check_resp_succeeded(resp)
        columns = ["range(5,1,2)"]
        self.check_column_names(resp, columns)
        expect_result = [[[]]]
        self.check_result(resp, expect_result)

        query = 'YIELD range(5,1,-2)'
        resp = self.execute_query(query)
        self.check_resp_succeeded(resp)
        columns = ["range(5,1,-(2))"]
        self.check_column_names(resp, columns)
        expect_result = [[[]]]
        self.check_result(resp, expect_result)

        query = 'YIELD range(5,1,0)'
        resp = self.execute_query(query)
        self.check_resp_succeeded(resp)
        columns = ["range(5,1,0)"]
        self.check_column_names(resp, columns)
        expect_result = [[[]]]
        self.check_result(resp, expect_result)
