#include "BehaviorTree.hpp"

SelectorNode::SelectorNode(std::vector<BTNodePtr> children)
    : m_children(std::move(children)) {}

BTResult SelectorNode::tick(const BTContext& context) const {
    for (const auto& child : m_children) {
        BTResult result = child->tick(context);
        if (result.success) {
            return result;
        }
    }
    return BTResult{ false, "" };
}

SequenceNode::SequenceNode(std::vector<BTNodePtr> children)
    : m_children(std::move(children)) {}

BTResult SequenceNode::tick(const BTContext& context) const {
    std::string command;
    for (const auto& child : m_children) {
        BTResult result = child->tick(context);
        if (!result.success) {
            return BTResult{ false, "" };
        }
        if (!result.command.empty()) {
            command = result.command;
        }
    }
    return BTResult{ true, command };
}

ConditionNode::ConditionNode(std::string name)
    : m_name(std::move(name)) {}

BTResult ConditionNode::tick(const BTContext& context) const {
    for (const auto& fact : context) {
        if (fact.first == m_name && fact.second != 0) {
            return BTResult{ true, "" };
        }
    }
    return BTResult{ false, "" };
}

ActionNode::ActionNode(std::string command)
    : m_command(std::move(command)) {}

BTResult ActionNode::tick(const BTContext&) const {
    return BTResult{ true, m_command };
}

BehaviorTree::BehaviorTree(BTNodePtr root)
    : m_root(std::move(root)) {}

std::string BehaviorTree::tick(const BTContext& context) const {
    if (!m_root) {
        return "";
    }
    BTResult result = m_root->tick(context);
    return result.success ? result.command : "";
}
