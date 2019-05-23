#include "expression.h"
#include "cmake-build-debug/lexer.h"
#include "cmake-build-debug/parser.h"

#include <bits/stdc++.h>

using namespace std;

extern expression *result;
extern int yydebug;

void yyerror(const char *error) {
    cerr << error << endl;
    cerr << "ERROR AT LINE: " << endl;
}

int yywrap() { return 1; }

shared_ptr<expression> root;

void make_expression(string const &s) {
    yy_scan_string(s.c_str());
    yyparse();
    root = shared_ptr<expression>(result);
}

int expression::counter = 0, expression::cnt = 0, type::cntr = 0;
unordered_map<string, string> expression::new_name;
unordered_map<string, shared_ptr<type>> expression::types;
vector<equality> expression::system_of_expr;
unordered_map<string, shared_ptr<type>> expression::unificator;

int main() {
    yydebug = 0;
    freopen("../input.txt", "r", stdin);
    ios_base::sync_with_stdio(false);

    string str;
    string input;

    expression::new_name = unordered_map<string,string>();
    expression::types = unordered_map<string, shared_ptr<type>> ();
    expression::system_of_expr = vector<equality> ();
    expression::unificator = unordered_map<string, shared_ptr<type>> ();

    while (getline(cin, str)) {
        input += str +
                '\n';
    }

    make_expression(input);
    root->rename();
    auto t  = root->W();
    cout << t.second->to_string();

    return 0;
}
