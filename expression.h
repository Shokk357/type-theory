#ifndef EXPR
#define EXPR

#include <bits/stdc++.h>

using namespace std;

struct type {
    static int cntr;

    virtual string to_string() = 0;

    virtual ~type() = default;

    virtual shared_ptr<type> replace(unordered_map<string, shared_ptr<type>> &substitution) = 0;

    virtual unordered_set<string> find_free_var() = 0;

    virtual shared_ptr<type> remove_forall() = 0;
};

struct binary : type {
    shared_ptr<type> left;
    shared_ptr<type> right;

    binary(shared_ptr<type> const &left, shared_ptr<type> const &right) {
        this->left = left;
        this->right = right;
    }

};

struct implication : binary {

    implication(shared_ptr<type> const &left, shared_ptr<type> const &right) : binary(left, right) {}

    string to_string() override {
        return "(" + left->to_string() + " -> " + right->to_string() + ")";
    }

    shared_ptr<type> replace(unordered_map<string, shared_ptr<type>> &substitution) override {
        return shared_ptr<type>(new implication(left->replace(substitution), right->replace(substitution)));
    }

    unordered_set<string> find_free_var() override {
        auto a = left->find_free_var();
        auto b = right->find_free_var();
        a.insert(b.begin(), b.end());
        return a;
    }

    shared_ptr<type> remove_forall() override {
        return shared_ptr<type>(new implication(left->remove_forall(), right->remove_forall()));
    }
};

struct type_variable : type {
    string id;

    explicit type_variable(string s) {
        id = s;
    }

    string to_string() override {
        return id;
    }

    ~type_variable() override = default;

    shared_ptr<type> replace(unordered_map<string, shared_ptr<type>> &substitution) override {
        if (substitution.count(id)) {
            return substitution[id]->replace(substitution);
        }
        return shared_ptr<type>(new type_variable(id));
    }

    unordered_set<string> find_free_var() override {
        return {id};
    }

    shared_ptr<type> remove_forall() override {
        return shared_ptr<type>(new type_variable(id));
    }
};

struct forall : binary {
    shared_ptr<type> id;
    shared_ptr<type> expr;

    forall(shared_ptr<type> const &id, shared_ptr<type> const &expr) : binary(id, expr) {
        this->id = left;
        this->expr = right;
    }

    string to_string() override {
        return "(forall " + id->to_string() + "." + expr->to_string() + ")";
    }

    shared_ptr<type> replace(unordered_map<string, shared_ptr<type>> &substitution) override {
        return shared_ptr<type>(new forall(id, expr->replace(substitution)));
    }

    unordered_set<string> find_free_var() override {
        auto res = expr->find_free_var();
        res.erase(id->to_string());
        return res;
    }

    shared_ptr<type> remove_forall() override {
        unordered_map<string, shared_ptr<type>> sub;
        sub[id->to_string()] = shared_ptr<type>(new type_variable("t" + std::to_string(cntr++)));
        return (expr->replace(sub))->remove_forall();
    }
};

struct equality {
    shared_ptr<type> left, right;

    equality(shared_ptr<type> left, shared_ptr<type> right) {
        this->left = left;
        this->right = right;
    }

    string to_string() {
        return "(" + left->to_string() + " = " + right->to_string() + ")";
    }

    ~equality() = default;
};

struct expression {
    static int counter, cnt;
    static unordered_map<string, string> new_name;
    static unordered_map<string, shared_ptr<type>> types;
    static vector<equality> system_of_expr;
    static unordered_map<string, shared_ptr<type>> unificator;


    void unificate() {
        unificator.clear();

        while (!system_of_expr.empty()) {
            unif();
        }
    }

    unordered_map<string, shared_ptr<type>>
    composition(unordered_map<string, shared_ptr<type>> &s1, unordered_map<string, shared_ptr<type>> &s2) {
        unordered_map<string, shared_ptr<type>> res;

        for (auto &entry : s2) {
            res[entry.first] = entry.second;
        }

        for (auto &entry : s1) {
            res[entry.first] = entry.second->replace(s2);
        }

        return res;
    }

    bool check_variable(string s, shared_ptr<type> t) {
        auto first = dynamic_pointer_cast<binary>(t);
        if (first != nullptr) {
            return check_variable(s, first->left) || check_variable(s, first->right);
        }
        auto second = dynamic_pointer_cast<type_variable>(t);
        return second->id == s;
    }

    void unif() {
        for (int i = 0; i < system_of_expr.size(); ++i) {
            auto v = system_of_expr[i];
            v = equality(v.left->replace(unificator), v.right->replace(unificator));
            auto left = dynamic_pointer_cast<type_variable>(v.left);
            if (left != nullptr) {
                auto right = dynamic_pointer_cast<type_variable>(v.right);
                if (right != nullptr) {
                    if (left->id != right->id) {
                        unificator[left->id] = v.right;
                        swap(system_of_expr[i], system_of_expr.back());
                        system_of_expr.pop_back();
                    } else {
                        swap(system_of_expr[i], system_of_expr.back());
                        system_of_expr.pop_back();
                    }
                } else {
                    if (check_variable(left->id, v.right)) {
                        cout << "Expression has no type" << endl;
                        exit(101);
                    } else {
                        unificator[left->id] = v.right;
                        swap(system_of_expr[i], system_of_expr.back());
                        system_of_expr.pop_back();
                    }
                }
            } else {
                auto right = dynamic_pointer_cast<type_variable>(v.right);
                if (right != nullptr) {
                    unificator[right->id] = v.left;
                    swap(system_of_expr[i], system_of_expr.back());
                    system_of_expr.pop_back();
                } else {
                    system_of_expr.emplace_back(dynamic_pointer_cast<binary>(v.left)->left,
                                                dynamic_pointer_cast<binary>(v.right)->left);
                    system_of_expr.emplace_back(dynamic_pointer_cast<binary>(v.left)->right,
                                                dynamic_pointer_cast<binary>(v.right)->right);
                    swap(system_of_expr[i], system_of_expr.back());
                    system_of_expr.pop_back();
                }
            }
        }
    }

    virtual string to_string() = 0;

    virtual ~expression() = default;

    virtual void rename() = 0;

    virtual pair<unordered_map<string, shared_ptr<type>>, shared_ptr<type>> W() = 0;
};

struct variable : expression {
    string id;

    explicit variable(string s) {
        id = std::move(s);
    }

    explicit variable(string *s) : variable(*s) {
        delete s;
    }

    string to_string() override {
        return id;
    }

    void rename() override {
        if (new_name.count(id) == 0) {
            new_name[id] = "pavlik" + std::to_string(counter++);
        }
        id = new_name[id];
    }

    pair<unordered_map<string, shared_ptr<type>>, shared_ptr<type>> W() override {
        auto t = types.count(id) != 0 ? types[id] : shared_ptr<type>(new type_variable("type" + std::to_string(cnt++)));
        t = t->remove_forall();
        return make_pair(unordered_map<string, shared_ptr<type>>(), t);
    }
};

struct abstraction : expression {
    string var;
    shared_ptr<expression> expr;

    abstraction(string *var, expression *expr) {
        this->var = *var;
        delete (var);
        this->expr = shared_ptr<expression>(expr);
    }

    string to_string() override {
        return "(\\" + var + "." + expr->to_string() + ")";
    }

    void rename() override {
        if (new_name.count(var)) {
            string old = new_name[var];
            new_name[var] = "pavlik" + std::to_string(counter++);
            string oldId = var;
            var = new_name[var];
            expr->rename();
            new_name[oldId] = old;
        } else {
            new_name[var] = "pavlik" + std::to_string(counter++);
            string oldId = var;
            var = new_name[var];
            expr->rename();
            new_name.erase(oldId);
        }
    }

    pair<unordered_map<string, shared_ptr<type>>, shared_ptr<type>> W() override {
        auto t = shared_ptr<type>(new type_variable("type" + std::to_string(cnt++)));
        bool f = types.count(var) != 0;
        auto old = f ? types[var] : nullptr;
        types[var] = t;
        auto expr_res = expr->W();
        if (f) {
            types[var] = old;
        } else {
            types.erase(var);
        }
        return make_pair(expr_res.first,
                         shared_ptr<type>(new implication(t->replace(expr_res.first), expr_res.second)));
    }

};

struct application : expression {
    shared_ptr<expression> left;
    shared_ptr<expression> right;

    application(expression *left, expression *right) {
        this->left = shared_ptr<expression>(left);
        this->right = shared_ptr<expression>(right);
    }

    string to_string() override {
        return "(" + left->to_string() + " " + right->to_string() + ")";
    }

    void rename() override {
        left->rename();
        right->rename();
    }

    pair<unordered_map<string, shared_ptr<type>>, shared_ptr<type>> W() override {
        auto left_res = left->W();
        auto old = types;
        types = composition(types, left_res.first);
        auto right_res = right->W();
        types = old;
        auto t = shared_ptr<type>(new type_variable("type" + std::to_string(cnt++)));
        system_of_expr.clear();
        system_of_expr.emplace_back(equality(left_res.second->replace(right_res.first),
                                             shared_ptr<type>(new implication(right_res.second, t))));
        unificate();

        auto tmp_s = composition(left_res.first, right_res.first);
        auto res_s = composition(tmp_s, unificator);

        return make_pair(res_s, t->replace(unificator));
    }

};

struct let : expression {
    string id;
    shared_ptr<expression> substitution;
    shared_ptr<expression> expr;

    let(string *id, expression *substitution, expression *expr) {
        this->id = *id;
        delete (id);
        this->substitution = shared_ptr<expression>(substitution);
        this->expr = shared_ptr<expression>(expr);
    }

    string to_string() override {
        return "(let " + id + " = " + substitution->to_string() + " in " + expr->to_string() + ")";
    }

    void rename() override {
        if (new_name.count(id)) {
            string old = new_name[id];
            new_name[id] = "pavlik" + std::to_string(counter++);;
            string oldId = id;
            id = new_name[id];
            expr->rename();
            new_name[oldId] = old;
        } else {
            new_name[id] = "pavlik" + std::to_string(counter++);;
            string oldId = id;
            id = new_name[id];
            expr->rename();
            new_name.erase(oldId);
        }
        substitution->rename();
    }

    pair<unordered_map<string, shared_ptr<type>>, shared_ptr<type>> W() override {
        auto res_sub = substitution->W();
        auto old_types = types;
        auto t = res_sub.second;
        auto free_vars = t->find_free_var();
        for (auto &x : free_vars) {
            t = shared_ptr<type>(new forall(shared_ptr<type>(new type_variable(x)), t));
        }

        types[id] = t;
        types = composition(types, res_sub.first);
        auto res_expr = expr->W();
        types = old_types;
        return make_pair(composition(res_expr.first, res_sub.first), res_expr.second);
    }

};

#endif