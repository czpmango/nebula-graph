/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#ifndef PARSER_CYPHERSENTENCE_H_
#define PARSER_CYPHERSENTENCE_H_

#include "common/expression/ContainerExpression.h"
#include "common/expression/SubscriptExpression.h"
#include "parser/Clauses.h"
#include "parser/Sentence.h"
#include "parser/TraverseSentences.h"

namespace nebula {

class PatternElement {
public:
    explicit PatternElement(std::string alias = "", bool isSingleNode = false)
        : alias_(alias), isSingleNode_(isSingleNode) {}

    std::string alias() const {
        return alias_;
    }

    void setAlias(std::string alias) {
        alias_ = alias;
    }

    std::unordered_set<std::string> allAliases() const {
        return allAliases_;
    }

    bool isSingleNode() const {
        return isSingleNode_;
    }

    virtual std::unique_ptr<PatternElement> clone() const = 0;
    virtual std::string toString() const = 0;
    virtual ~PatternElement() = default;

protected:
    PatternElement(const PatternElement& elem) {
        alias_ = elem.alias();
        allAliases_ = elem.allAliases();
        isSingleNode_ = elem.isSingleNode();
    }

    PatternElement& operator=(PatternElement& e) = delete;

protected:
    std::string alias_;
    std::unordered_set<std::string> allAliases_;
    bool isSingleNode_{false};
};

class NodePattern final : public PatternElement {
public:
    NodePattern(std::string alias, std::vector<std::string> labels, Expression* props)
        : PatternElement(alias, true) {
        labels_ = std::move(labels);
        if (props) {
            DCHECK_EQ(props->kind(), Expression::Kind::kMap);
            props_.reset(static_cast<MapExpression*>(props));
        }
    }

    NodePattern(const NodePattern& nodePattern) : PatternElement(nodePattern) {
        labels_ = nodePattern.labels();
        props_.reset(static_cast<MapExpression*>(
            nodePattern.props() ? nodePattern.props()->clone().release() : nullptr));
    }

    NodePattern& operator=(const NodePattern& nodePattern) {
        alias_ = nodePattern.alias();
        allAliases_ = nodePattern.allAliases();
        isSingleNode_ = nodePattern.isSingleNode();
        labels_ = nodePattern.labels();
        props_.reset(static_cast<MapExpression*>(
            nodePattern.props() ? nodePattern.props()->clone().release() : nullptr));
        return *this;
    }

    std::vector<std::string> labels() const {
        return labels_;
    }

    MapExpression* props() const {
        return props_.get();
    }

    std::unique_ptr<PatternElement> clone() const override {
        return std::make_unique<NodePattern>(
            alias_, labels_, props_ ? props_->clone().release() : nullptr);
    }

    std::string toString() const override;

private:
    std::vector<std::string> labels_{};
    std::unique_ptr<MapExpression> props_;
};

class EdgePattern {
public:
    using Direction = nebula::storage::cpp2::EdgeDirection;
    EdgePattern(std::string alias,
                std::vector<std::string> edgeTypes,
                Expression* props,
                std::pair<int64_t, int64_t> range,
                EdgePattern::Direction direction = {EdgePattern::Direction::OUT_EDGE}) {
        alias_ = alias;
        edgeTypes_ = std::move(edgeTypes);
        if (props) {
            DCHECK_EQ(props->kind(), Expression::Kind::kMap);
            props_.reset(static_cast<MapExpression*>(props));
        }
        range_ = range;
        direction_ = direction;
    }

    EdgePattern(const EdgePattern& edgePattern) {
        alias_ = edgePattern.alias();
        edgeTypes_ = edgePattern.edgeTypes();
        range_ = edgePattern.range();
        if (edgePattern.props()) {
            props_.reset(static_cast<MapExpression*>(edgePattern.props()));
        }
        direction_ = edgePattern.direction();
    }

    EdgePattern& operator=(const EdgePattern& edgePattern) {
        alias_ = edgePattern.alias();
        edgeTypes_ = edgePattern.edgeTypes();
        range_ = edgePattern.range();
        if (edgePattern.props()) {
            props_.reset(static_cast<MapExpression*>(edgePattern.props()));
        }
        direction_ = edgePattern.direction();
        return *this;
    }

    std::string alias() const {
        return alias_;
    }

    std::vector<std::string> edgeTypes() const {
        return edgeTypes_;
    }

    std::pair<int64_t, int64_t> range() const {
        return range_;
    }

    MapExpression* props() const {
        return props_.get();
    }

    EdgePattern::Direction direction() const {
        return direction_;
    }

    void setDirection(EdgePattern::Direction direction) {
        direction_ = direction;
    }

    std::unique_ptr<EdgePattern> clone() const {
        return std::make_unique<EdgePattern>(
            alias_, edgeTypes_, props_ ? props_->clone().release() : nullptr, range_, direction_);
    }

    std::string toString() const;

private:
    std::string alias_;
    std::vector<std::string> edgeTypes_{};
    std::pair<int64_t, int64_t> range_{1, 1};
    std::unique_ptr<MapExpression> props_;
    EdgePattern::Direction direction_{EdgePattern::Direction::OUT_EDGE};
};

class PathPattern final : public PatternElement {
public:
    PathPattern(PatternElement* element,
                EdgePattern* edge = nullptr,
                NodePattern* rightNode = nullptr,
                std::string alias = "")
        : PatternElement(alias, false) {
        element_.reset(element);
        edge_.reset(edge);
        rightNode_.reset(rightNode);
    }

    PathPattern(const PathPattern& pathPattern) : PatternElement(pathPattern) {
        element_.reset(static_cast<PatternElement*>(pathPattern.element()->clone().release()));
        edge_.reset(static_cast<EdgePattern*>(
            pathPattern.edge() ? pathPattern.edge()->clone().release() : nullptr));
        rightNode_.reset(static_cast<NodePattern*>(
            pathPattern.rightNode() ? pathPattern.rightNode()->clone().release() : nullptr));
    }

    PathPattern& operator=(const PathPattern& pathPattern) {
        alias_ = pathPattern.alias();
        allAliases_ = pathPattern.allAliases();
        isSingleNode_ = pathPattern.isSingleNode();
        element_.reset(static_cast<PatternElement*>(pathPattern.element()->clone().release()));
        edge_.reset(static_cast<EdgePattern*>(
            pathPattern.edge() ? pathPattern.edge()->clone().release() : nullptr));
        rightNode_.reset(static_cast<NodePattern*>(
            pathPattern.rightNode() ? pathPattern.rightNode()->clone().release() : nullptr));
        return *this;
    }

    PatternElement* element() const {
        return element_.get();
    }

    EdgePattern* edge() const {
        return edge_.get();
    }

    void setEdge(EdgePattern* edge) {
        edge_.reset(edge);
    }

    NodePattern* rightNode() const {
        return rightNode_.get();
    }

    void setRightNode(NodePattern* rightNode) {
        rightNode_.reset(rightNode);
    }

    std::unique_ptr<PatternElement> clone() const override {
        return std::make_unique<PathPattern>(
            element_->clone().release(),
            static_cast<EdgePattern*>(edge_->clone().release()),
            static_cast<NodePattern*>(rightNode_->clone().release()),
            alias_);
    }

    std::string toString() const override;

private:
    std::unique_ptr<PatternElement> element_;
    std::unique_ptr<EdgePattern> edge_;
    std::unique_ptr<NodePattern> rightNode_;
};

class Pattern final {
public:
    explicit Pattern(std::vector<PatternElement> patternElements = {})
        : patternElements_(std::move(patternElements)) {}

    std::vector<PatternElement>& patternElements() {
        return patternElements_;
    }

    std::string toString() const;

private:
    std::vector<PatternElement> patternElements_;
};

class LabelList final {
public:
    explicit LabelList(std::vector<std::string> items) {
        items_ = items;
    }

    std::vector<std::string> items() {
        return items_;
    }

    void addItem(std::string item) {
        items_.emplace_back(item);
    }

    std::string toString() const;

private:
    std::vector<std::string> items_;
};

class CypherClause {
public:
    enum class Kind : uint8_t {
        kMatch,
        kUnwind,
        kWith,
        // kReturn,
    };
    explicit CypherClause(Kind kind) {
        kind_ = kind;
    }
    virtual ~CypherClause() = default;

    auto kind() const {
        return kind_;
    }

    bool isMatch() const {
        return kind() == Kind::kMatch;
    }

    bool isUnwind() const {
        return kind() == Kind::kUnwind;
    }

    bool isWith() const {
        return kind() == Kind::kWith;
    }

    virtual std::string toString() const = 0;

private:
    Kind kind_;
};

class ReturnClause final : public CypherClause {
public:
    explicit ReturnClause(YieldColumns* columns = nullptr,
                          OrderFactors* orderFactors = nullptr,
                          Expression* skip = nullptr,
                          Expression* limit = nullptr,
                          bool distinct = false)
        : CypherClause(Kind::kMatch) {
        columns_.reset(columns);
        orderFactors_.reset(orderFactors);
        skip_.reset(skip);
        limit_.reset(limit);
        isDistinct_ = distinct;
        if (columns_ == nullptr) {
            isAll_ = true;
        }
    }

    const YieldColumns* columns() const {
        return columns_.get();
    }

    void setColumns(YieldColumns* columns) {
        columns_.reset(columns);
    }

    bool isAll() const {
        return isAll_;
    }

    bool isDistinct() const {
        return isDistinct_;
    }

    const Expression* skip() const {
        return skip_.get();
    }

    const Expression* limit() const {
        return limit_.get();
    }

    OrderFactors* orderFactors() {
        return orderFactors_.get();
    }

    const OrderFactors* orderFactors() const {
        return orderFactors_.get();
    }

    std::string toString() const;

private:
    std::unique_ptr<YieldColumns> columns_;
    bool isAll_{false};
    bool isDistinct_{false};
    std::unique_ptr<OrderFactors> orderFactors_;
    std::unique_ptr<Expression> skip_;
    std::unique_ptr<Expression> limit_;
};

class MatchClause final : public CypherClause {
public:
    MatchClause(PathPattern* path, WhereClause* where, bool optional) : CypherClause(Kind::kMatch) {
        path_.reset(path);
        where_.reset(where);
        isOptional_ = optional;
    }

    PathPattern* path() {
        return path_.get();
    }

    const PathPattern* path() const {
        return path_.get();
    }

    WhereClause* where() {
        return where_.get();
    }

    const WhereClause* where() const {
        return where_.get();
    }

    bool isOptional() const {
        return isOptional_;
    }

    std::string toString() const override;

private:
    bool isOptional_{false};
    std::unique_ptr<PathPattern> path_;
    std::unique_ptr<WhereClause> where_;
};

class UnwindClause final : public CypherClause {
public:
    UnwindClause(Expression* expr, std::string* alias) : CypherClause(Kind::kUnwind) {
        expr_.reset(expr);
        alias_.reset(alias);
    }

    Expression* expr() {
        return expr_.get();
    }

    const Expression* expr() const {
        return expr_.get();
    }

    std::string* alias() {
        return alias_.get();
    }

    const std::string* alias() const {
        return alias_.get();
    }

    std::string toString() const override;

private:
    std::unique_ptr<Expression> expr_;
    std::unique_ptr<std::string> alias_;
};

class WithClause final : public CypherClause {
public:
    explicit WithClause(YieldColumns* cols,
                        OrderFactors* orderFactors = nullptr,
                        Expression* skip = nullptr,
                        Expression* limit = nullptr,
                        WhereClause* where = nullptr,
                        bool distinct = false)
        : CypherClause(Kind::kWith) {
        columns_.reset(cols);
        orderFactors_.reset(orderFactors);
        skip_.reset(skip);
        limit_.reset(limit);
        where_.reset(where);
        isDistinct_ = distinct;
    }

    YieldColumns* columns() {
        return columns_.get();
    }

    const YieldColumns* columns() const {
        return columns_.get();
    }

    OrderFactors* orderFactors() {
        return orderFactors_.get();
    }

    const OrderFactors* orderFactors() const {
        return orderFactors_.get();
    }

    Expression* skip() {
        return skip_.get();
    }

    const Expression* skip() const {
        return skip_.get();
    }

    Expression* limit() {
        return limit_.get();
    }

    const Expression* limit() const {
        return limit_.get();
    }

    WhereClause* where() {
        return where_.get();
    }

    const WhereClause* where() const {
        return where_.get();
    }

    bool isDistinct() const {
        return isDistinct_;
    }

    std::string toString() const override;

private:
    std::unique_ptr<YieldColumns> columns_;
    std::unique_ptr<OrderFactors> orderFactors_;
    std::unique_ptr<Expression> skip_;
    std::unique_ptr<Expression> limit_;
    std::unique_ptr<WhereClause> where_;
    bool isDistinct_;
};

// TODO: delete MatchClauseList
class MatchClauseList final {
public:
    void add(CypherClause* clause) {
        clauses_.emplace_back(clause);
    }

    void add(MatchClauseList* list) {
        DCHECK(list != nullptr);
        for (auto& clause : list->clauses_) {
            clauses_.emplace_back(std::move(clause));
        }
        delete list;
    }

    auto clauses() && {
        return std::move(clauses_);
    }

private:
    std::vector<std::unique_ptr<CypherClause>> clauses_;
};

class CypherSentence final : public Sentence {
public:
    CypherSentence(MatchClauseList* clauses, ReturnClause* ret) : Sentence(Kind::kMatch) {
        clauses_ = std::move(*clauses).clauses();
        delete clauses;
        return_.reset(ret);
    }

    auto& clauses() {
        return clauses_;
    }

    const auto& clauses() const {
        return clauses_;
    }

    const ReturnClause* ret() const {
        return return_.get();
    }

    ReturnClause* ret() {
        return return_.get();
    }

    std::string toString() const override;

private:
    std::vector<std::unique_ptr<CypherClause>> clauses_;
    std::unique_ptr<ReturnClause> return_;
};

class MatchPath final {
    // TODO: delete this class and use PathPattern after refactor cypher validator
public:
    explicit MatchPath(const PathPattern& pattern) : alias_(pattern.alias()), pattern_(pattern) {
        decomposePattern(pattern_.element(), pattern_.edge(), pattern_.rightNode());
    }

    // decompose pattern into nodes_/edges_
    void decomposePattern(PatternElement* elem, EdgePattern* edge, NodePattern* rightNode) {
        if (edge) {
            edges_.emplace_back(*edge);
        }
        if (rightNode) {
            nodes_.emplace_back(*rightNode);
        }
        if (elem) {
            if (!elem->isSingleNode()) {
                auto* pathPattern = static_cast<PathPattern*>(elem);
                decomposePattern(
                    pathPattern->element(), pathPattern->edge(), pathPattern->rightNode());
            } else {
                nodes_.emplace_back(*static_cast<NodePattern*>(elem));
            }
        }
    }

    void setAlias(std::string alias) {
        alias_ = alias;
    }

    const std::string alias() const {
        return alias_;
    }

    const auto& nodes() const {
        return nodes_;
    }

    const auto& edges() const {
        return edges_;
    }

    size_t steps() const {
        return edges_.size();
    }

    const NodePattern& node(size_t i) const {
        return nodes_[i];
    }

    const EdgePattern& edge(size_t i) const {
        return edges_[i];
    }

    std::string toString() const;

private:
    std::string alias_;
    PathPattern pattern_;
    std::vector<NodePattern> nodes_;
    std::vector<EdgePattern> edges_;
};

}   // namespace nebula

#endif   // PARSER_CYPHERSENTENCE_H_