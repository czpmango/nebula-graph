/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include "parser/CypherSentence.h"

namespace nebula {

std::string CypherSentence::toString() const {
    std::string buf;
    buf.reserve(256);

    for (auto &clause : clauses_) {
        buf += clause->toString();
        buf += " ";
    }

    buf += return_->toString();

    return buf;
}

std::string ReturnClause::toString() const {
    std::string buf;
    buf.reserve(64);

    buf += "RETURN ";

    if (isDistinct()) {
        buf += "DISTINCT ";
    }

    if (isAll()) {
        buf += '*';
    } else {
        buf += columns_->toString();
    }

    if (orderFactors_ != nullptr) {
        buf += " ";
        buf += "ORDER BY ";
        buf += orderFactors_->toString();
    }

    if (skip_ != nullptr) {
        buf += " ";
        buf += "SKIP ";
        buf += skip_->toString();
    }

    if (limit_ != nullptr) {
        buf += " ";
        buf += "LIMIT ";
        buf += limit_->toString();
    }

    return buf;
}

std::string MatchClause::toString() const {
    std::string buf;
    buf.reserve(256);

    if (isOptional()) {
        buf += "OPTIONAL ";
    }

    buf += "MATCH ";
    buf += path_->toString();
    if (where_ != nullptr) {
        buf += " ";
        buf += where_->toString();
    }
    return buf;
}

std::string UnwindClause::toString() const {
    std::string buf;
    buf.reserve(256);

    buf += "UNWIND ";
    buf += expr_->toString();
    buf += " AS ";
    buf += alias_;

    return buf;
}

std::string WithClause::toString() const {
    std::string buf;
    buf.reserve(256);

    buf += "WITH ";

    if (isDistinct()) {
        buf += "DISTINCT ";
    }

    buf += columns_->toString();

    if (orderFactors_ != nullptr) {
        buf += " ";
        buf += "ORDER BY ";
        buf += orderFactors_->toString();
    }

    if (skip_ != nullptr) {
        buf += " ";
        buf += "SKIP ";
        buf += skip_->toString();
    }

    if (limit_ != nullptr) {
        buf += " ";
        buf += "LIMIT ";
        buf += limit_->toString();
    }

    if (where_ != nullptr) {
        buf += " ";
        buf += where_->toString();
    }

    return buf;
}

std::string EdgePattern::toString() const {
    std::string buf;
    buf.reserve(256);

    std::string end;
    if (direction_ == Direction::OUT_EDGE) {
        buf += '-';
        end = "->";
    } else if (direction_ == Direction::IN_EDGE) {
        buf += "<-";
        end = "-";
    } else {
        buf += '-';
        end = "-";
    }

    buf += '[';
    if (!alias_.empty()) {
        buf += alias_;
    }
    if (!edgeTypes_.empty()) {
        buf += ':';
        buf += edgeTypes_[0];
        for (auto i = 1u; i < edgeTypes_.size(); i++) {
            buf += "|";
            buf += edgeTypes_[i];
        }
    }
    if (range_.first != 1 || range_.second != 1) {
        buf += "*";
        if (range_.first == range_.second) {
            buf += folly::to<std::string>(range_.first);
        } else if (range_.second == std::numeric_limits<int64_t>::max()) {
            if (range_.first != 1) {
                buf += folly::to<std::string>(range_.first);
                buf += "..";
            }
        } else {
            buf += folly::to<std::string>(range_.first);
            buf += "..";
            buf += folly::to<std::string>(range_.second);
        }
    }
    if (props_ != nullptr) {
        buf += props_->toString();
    }
    buf += ']';

    buf += end;

    return buf;
}

std::string NodePattern::toString() const {
    std::string buf;
    buf.reserve(64);

    buf += '(';
    buf += alias_;
    for (std::string label : labels_) {
        // TODO: fix this
        buf += label;
    }
    if (props_) {
        buf += props_->toString();
    }
    buf += ')';

    return buf;
}

std::string PathPattern::toString() const {
    std::string buf;
    buf.reserve(256);

    if (!alias_.empty()) {
        buf += alias_;
        buf += " = ";
    }

    if (element_) {
        buf += element_->toString();
    }
    if (edge_) {
        buf += edge_->toString();
    }
    if (rightNode_) {
        buf += rightNode_->toString();
    }

    return buf;
}

std::string MatchPath::toString() const {
    std::string buf;
    buf.reserve(256);

    if (!alias_.empty()) {
        buf += alias_;
        buf += " = ";
    }

    buf += node(0).toString();
    for (auto i = 0u; i < edges_.size(); i++) {
        buf += edge(i).toString();
        buf += node(i + 1).toString();
    }

    return buf;
}
}   // namespace nebula
