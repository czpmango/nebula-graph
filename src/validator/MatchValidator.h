/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#ifndef VALIDATOR_MATCHVALIDATOR_H_
#define VALIDATOR_MATCHVALIDATOR_H_

#include "common/base/Base.h"
#include "validator/TraversalValidator.h"
#include "util/AnonVarGenerator.h"
#include "planner/plan/Query.h"
#include "context/ast/CypherAstContext.h"

namespace nebula {

class MatchStepRange;

namespace graph {
class MatchValidator final : public TraversalValidator {
public:
    MatchValidator(Sentence *sentence, QueryContext *context);

private:
    Status validateImpl() override;

    AstContext* getAstContext() override;

    Status validatePath(const PathPattern *path, MatchClauseContext &matchClauseCtx) const;

    Status validateFilter(const Expression *filter, WhereClauseContext &whereClauseCtx) const;

    Status validateReturn(ReturnClause *ret,
                          const CypherClauseContextBase *cypherClauseCtx,
                          ReturnClauseContext &retClauseCtx) const;

    Status validateAliases(const std::vector<const Expression *> &exprs,
                           const std::unordered_map<std::string, AliasType> *aliases) const;

    Status validateStepRange(const MatchStepRange *range) const;

    Status validateWith(const WithClause *with, WithClauseContext &withClauseCtx) const;

    Status validateUnwind(const UnwindClause *unwind, UnwindClauseContext &unwindClauseCtx) const;

    Status validatePagination(const Expression *skipExpr,
                              const Expression *limitExpr,
                              PaginationContext &paginationCtx) const;

    Status validateOrderBy(const OrderFactors *factors,
                           const YieldColumns *yieldColumns,
                           OrderByClauseContext &orderByCtx) const;

    Status validateGroup(YieldClauseContext &yieldCtx) const;

    Status validateYield(YieldClauseContext &yieldCtx) const;

    StatusOr<Expression*> makeSubFilter(const std::string &alias,
                                        const MapExpression *map,
                                        const std::string &label = "") const;
    StatusOr<Expression*> makeSubFilterWithoutSave(const std::string &alias,
                                                   const MapExpression *map,
                                                   const std::string &label = "") const;

    static Expression* andConnect(Expression *left, Expression *right);

    template <typename T>
    T* saveObject(T *obj) const {
        return qctx_->objPool()->add(obj);
    }

    Status buildNodeInfo(const PathPattern *path,
                         std::vector<NodeInfo> &edgeInfos,
                         std::unordered_map<std::string, AliasType> &aliases) const;

    Status buildEdgeInfo(const PathPattern *path,
                         std::vector<EdgeInfo> &nodeInfos,
                         std::unordered_map<std::string, AliasType> &aliases) const;

    Status buildPathExpr(const PathPattern *path, MatchClauseContext &matchClauseCtx) const;

    Status combineAliases(std::unordered_map<std::string, AliasType> &curAliases,
                          const std::unordered_map<std::string, AliasType> &lastAliases) const;

    Status combineYieldColumns(YieldColumns *yieldColumns, YieldColumns *prevYieldColumns) const;

    StatusOr<AliasType> getAliasType(const std::unordered_map<std::string, AliasType> *aliasesUsed,
                                     const std::string *name) const;

    Status checkAlias(const Expression *refExpr,
                      const std::unordered_map<std::string, AliasType> *aliasesUsed) const;

    Status buildOutputs(const YieldColumns *yields);

    template <typename T>
    std::unique_ptr<T> getContext() const {
        auto ctx = std::make_unique<T>();
        ctx->sentence = sentence_;
        ctx->qctx = qctx_;
        ctx->space = space_;
        return ctx;
    }

private:
    std::unique_ptr<MatchAstContext>            matchCtx_;
};

}   // namespace graph
}   // namespace nebula

#endif  // VALIDATOR_MATCHVALIDATOR_H_
