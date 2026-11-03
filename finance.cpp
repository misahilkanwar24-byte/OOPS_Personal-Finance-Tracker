// Personal Finance Tracker and Automated Wealth Advisor - SAHIL KANWAR
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <stdexcept>
#include <cmath>
using namespace std;

template <typename T>                         // generic helper (template)
T clampValue(T v, T lo, T hi) { return v < lo ? lo : (v > hi ? hi : v); }

class InsufficientFunds : public runtime_error {       // custom exception
public:
    explicit InsufficientFunds(const string& m) : runtime_error(m) {}
};

class Transaction {
    string date, category, note; double amount; bool income;
    static int count;                                  // static member
public:
    Transaction(string d, string c, double a, bool inc, string n = "")
        : date(d), category(c), note(n), amount(a), income(inc) { ++count; }
    double getAmount() const { return amount; }
    bool isIncome() const { return income; }
    const string& getCategory() const { return category; }
    static int total() { return count; }
    friend ostream& operator<<(ostream& os, const Transaction& t) {   // overloading
        return os << t.date << "  " << left << setw(14) << t.category << right
                  << setw(9) << fixed << setprecision(0) << t.amount
                  << (t.income ? "  CR" : "  DR");
    }
};
int Transaction::count = 0;

class Account {                                        // abstract base class
protected:
    string name; double balance;
public:
    Account(string n, double b) : name(n), balance(b) {}
    virtual ~Account() {}
    void deposit(double a) { balance += a; }
    virtual void withdraw(double a) {
        if (a > balance) throw InsufficientFunds(name + ": balance too low");
        balance -= a;
    }
    virtual double yearlyRate() const = 0;             // pure virtual
    virtual string type() const = 0;
    double getBalance() const { return balance; }
    const string& getName() const { return name; }
};
class SavingsAccount : public Account {                // inheritance
    double minBalance;
public:
    SavingsAccount(string n, double b, double minB = 1000) : Account(n, b), minBalance(minB) {}
    void withdraw(double a) override {                 // overriding
        if (balance - a < minBalance) throw InsufficientFunds(name + ": minimum balance rule");
        balance -= a;
    }
    double yearlyRate() const override { return 0.04; }  string type() const override { return "Savings"; }
};
class WalletAccount : public Account {
public: using Account::Account;
    double yearlyRate() const override { return 0.0; }  string type() const override { return "Cash Wallet"; }
};

class Investment {                                     // abstract base class
protected:
    string name; double monthly;
public:
    Investment(string n, double m) : name(n), monthly(m) {}
    virtual ~Investment() {}
    virtual double annualRate() const = 0;
    virtual string risk() const = 0;
    virtual double futureValue(int years) const {      // monthly compounding
        double i = annualRate() / 12; int n = years * 12;
        return monthly * (pow(1 + i, n) - 1) / i * (1 + i);
    }
    double invested(int years) const { return monthly * years * 12; }
    const string& getName() const { return name; }
    double getMonthly() const { return monthly; }
};
class FixedDeposit : public Investment {
public: using Investment::Investment;
    double annualRate() const override { return 0.07; }  string risk() const override { return "Low"; }
};
class GoldSaving : public Investment {
public: using Investment::Investment;
    double annualRate() const override { return 0.08; }  string risk() const override { return "Medium"; }
};
class MutualFundSIP : public Investment {
public: using Investment::Investment;
    double annualRate() const override { return 0.12; }  string risk() const override { return "High"; }
};

class Advisor {                                        // strategy: abstract advisor
public:
    virtual ~Advisor() {}
    virtual string profile() const = 0;
    virtual double fdShare() const = 0;
    virtual double goldShare() const = 0;
    double sipShare() const { return 1.0 - fdShare() - goldShare(); }
};
class ConservativeAdvisor : public Advisor {
public: string profile() const override { return "Conservative"; }
    double fdShare() const override { return 0.60; }  double goldShare() const override { return 0.20; }
};
class BalancedAdvisor : public Advisor {
public: string profile() const override { return "Balanced"; }
    double fdShare() const override { return 0.30; }  double goldShare() const override { return 0.20; }
};
class AggressiveAdvisor : public Advisor {
public: string profile() const override { return "Aggressive"; }
    double fdShare() const override { return 0.10; }  double goldShare() const override { return 0.10; }
};
unique_ptr<Advisor> chooseAdvisor(int riskScore) {     // factory function
    if (riskScore <= 8) return make_unique<ConservativeAdvisor>();
    if (riskScore <= 14) return make_unique<BalancedAdvisor>();
    return make_unique<AggressiveAdvisor>();
}

class Goal {
    string name; double target; int years;
public:
    Goal(string n, double t, int y) : name(n), target(t), years(y) {}
    double requiredMonthly(double annualRate) const {
        double i = annualRate / 12; int n = years * 12;
        return target / ((pow(1 + i, n) - 1) / i * (1 + i));
    }
    string describe() const {
        return name + " (Rs " + to_string((int)target) + " in " + to_string(years) + " yr)";
    }
};

class FinanceTracker {
    string owner; vector<Transaction> tx; vector<unique_ptr<Account>> accounts;
    map<string, bool> isNeed;                          // category -> need / want
public:
    FinanceTracker(string o) : owner(o) {
        for (string c : {"Rent", "Groceries", "Utilities", "Transport", "EMI", "Health", "Education"})
            isNeed[c] = true;
        for (string c : {"Dining", "Shopping", "Entertainment"}) isNeed[c] = false;
    }
    void addAccount(unique_ptr<Account> a) { accounts.push_back(move(a)); }
    void add(const Transaction& t) { tx.push_back(t); }
    double income() const { double s = 0; for (auto& t : tx) if (t.isIncome()) s += t.getAmount(); return s; }
    double expense() const { double s = 0; for (auto& t : tx) if (!t.isIncome()) s += t.getAmount(); return s; }
    double needs() const {
        double s = 0;
        for (auto& t : tx) if (!t.isIncome() && isNeed.at(t.getCategory())) s += t.getAmount();
        return s;
    }
    double wants() const { return expense() - needs(); }
    double savings() const { return income() - expense(); }
    double liquid() const { double s = 0; for (auto& a : accounts) s += a->getBalance(); return s; }
    map<string, double> byCategory() const {
        map<string, double> m;
        for (auto& t : tx) if (!t.isIncome()) m[t.getCategory()] += t.getAmount();
        return m;
    }
    double healthScore() const {                       // 0 to 100
        double sr = clampValue(savings() / income() / 0.20, 0.0, 1.0) * 40;
        double ef = clampValue(liquid() / expense() / 6.0, 0.0, 1.0) * 30;
        double nr = needs() / income();
        double nd = nr <= 0.5 ? 30.0 : clampValue(30 * 0.5 / nr, 0.0, 30.0);
        return sr + ef + nd;
    }
    void report() const {
        double in = income();
        cout << fixed << setprecision(1);
        cout << "Income        : Rs " << setprecision(0) << in << "\nExpenses      : Rs " << expense()
             << "\nSavings       : Rs " << savings() << setprecision(1)
             << "  (" << 100 * savings() / in << "%)\n";
        cout << "Needs  (50%)  : " << 100 * needs() / in << "%\nWants  (30%)  : " << 100 * wants() / in
             << "%\nSaving (20%)  : " << 100 * savings() / in << "%\n";
        cout << "Emergency fund: " << setprecision(1) << liquid() / expense() << " months (target 6)\n";
    }
    void advise(const Advisor& adv, int years) const {
        double surplus = savings(), emergency = 0;
        if (liquid() / expense() < 6) emergency = surplus * 0.5;
        double invest = surplus - emergency;
        cout << "Profile: " << adv.profile() << " | surplus Rs " << setprecision(0) << surplus
             << " | to emergency fund Rs " << emergency << " | to invest Rs " << invest << "\n";
        vector<unique_ptr<Investment>> plan;
        plan.push_back(make_unique<FixedDeposit>("Recurring Deposit", invest * adv.fdShare()));
        plan.push_back(make_unique<GoldSaving>("Digital Gold", invest * adv.goldShare()));
        plan.push_back(make_unique<MutualFundSIP>("Index Fund SIP", invest * adv.sipShare()));
        double total = 0, put = 0;
        cout << left << setw(20) << "Instrument" << setw(8) << "Risk" << setw(10) << "Monthly"
             << setw(11) << "Invested" << "Value after " << years << " yrs\n";
        for (auto& p : plan) {                          // polymorphic calls
            cout << left << setw(20) << p->getName() << setw(8) << p->risk() << setw(10) << p->getMonthly()
                 << setw(11) << p->invested(years) << p->futureValue(years) << "\n";
            total += p->futureValue(years); put += p->invested(years);
        }
        cout << "Total invested Rs " << put << " -> expected value Rs " << total
             << " (about Rs " << total / pow(1.06, years) << " in today's money at 6% inflation)\n";
    }
    void save(const string& file) const {              // file handling
        ofstream f(file); f << "category,amount,type\n";
        for (auto& t : tx)
            f << t.getCategory() << "," << t.getAmount() << "," << (t.isIncome() ? "CR" : "DR") << "\n";
    }
    const vector<Transaction>& all() const { return tx; }
};
