/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include "common/datatypes/Value.h"

#include "context/QueryExpressionContext.h"

namespace nebula {
namespace graph {
const Value& QueryExpressionContext::getVar(const std::string& var) const {
    if (ectx_ == nullptr) {
        return Value::kEmpty;
    }
    return ectx_->getValue(var);
}

const Value& QueryExpressionContext::getVersionedVar(const std::string& var,
                                                    int64_t version) const {
    if (ectx_ == nullptr) {
        return Value::kEmpty;
    }
    auto& result = ectx_->getHistory(var);
    auto size = result.size();
    if (version <= 0 && static_cast<size_t>(std::abs(version)) < size) {
        return result[size + version -1].value();
    } else if (version > 0 && static_cast<size_t>(version) <= size) {
        return result[version - 1].value();
    } else {
        return Value::kEmpty;
    }
}

const Value& QueryExpressionContext::getVarProp(const std::string& var,
                                               const std::string& prop) const {
    UNUSED(var);
    if (iter_ != nullptr) {
        return iter_->getColumn(prop);
    } else {
        return Value::kEmpty;
    }
}

Value QueryExpressionContext::getEdgeProp(const std::string& edge,
                                         const std::string& prop) const {
    if (iter_ != nullptr) {
        return iter_->getEdgeProp(edge, prop);
    } else {
        return Value::kEmpty;
    }
}

Value QueryExpressionContext::getSrcProp(const std::string& tag,
                                        const std::string& prop) const {
    if (iter_ != nullptr) {
        return iter_->getTagProp(tag, prop);
    } else {
        return Value::kEmpty;
    }
}

const Value& QueryExpressionContext::getDstProp(const std::string& tag,
                                               const std::string& prop) const {
    if (iter_ != nullptr) {
        return iter_->getTagProp(tag, prop);
    } else {
        return Value::kEmpty;
    }
}

const Value& QueryExpressionContext::getInputProp(const std::string& prop) const {
    if (iter_ != nullptr) {
        return iter_->getColumn(prop);
    } else {
        return Value::kEmpty;
    }
}

void QueryExpressionContext::setVar(const std::string& var, Value val) {
    if (ectx_ == nullptr) {
        LOG(ERROR) << "Execution context was not provided.";
        DCHECK_NOTNULL(ectx_);
        return;
    }
    ectx_->setValue(var, std::move(val));
}
}  // namespace graph
}  // namespace nebula