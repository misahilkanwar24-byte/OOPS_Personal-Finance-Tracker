#include "finance.cpp"

int main() {
    FinanceTracker ft("Demo User");
    ft.addAccount(make_unique<SavingsAccount>("Bank Savings", 85000));
    ft.addAccount(make_unique<WalletAccount>("Cash Wallet", 5000));
    ft.add(Transaction("01-10-2026", "Salary", 60000, true));
    const vector<pair<string, double>> spend = {{"Rent", 15000}, {"Groceries", 6500}, {"Utilities", 2800},
        {"Transport", 2500}, {"EMI", 5000}, {"Dining", 4200}, {"Shopping", 5500}, {"Entertainment", 2500},
        {"Health", 1500}, {"Education", 3000}};
    int day = 2;
    for (auto& s : spend) {
        string d = (day < 10 ? "0" : "") + to_string(day) + "-10-2026"; day++;
        ft.add(Transaction(d, s.first, s.second, false));
    }

    cout << "=== 1. TRANSACTIONS (first 5 of " << Transaction::total() << ") ===\n";
    for (int i = 0; i < 5; i++) cout << ft.all()[i] << "\n";
    cout << "\n=== 2. MONTHLY REPORT (October 2026) ===\n"; ft.report();
    cout << "\n=== 3. SPENDING BY CATEGORY ===\n";
    cout << setprecision(0);
    for (auto& kv : ft.byCategory())
        cout << left << setw(15) << kv.first << right << setw(7) << kv.second
             << "  " << string((int)(kv.second / 1000), '#') << "\n";
    cout << "\nFinancial health score: " << setprecision(1) << ft.healthScore() << " / 100\n";

    cout << "\n=== 4. WEALTH ADVISOR (5-year plan) ===\n";
    for (int risk : {6, 12, 17}) {
        auto adv = chooseAdvisor(risk);
        cout << "\nRisk questionnaire score " << risk << " -> ";
        ft.advise(*adv, 5);
    }
    cout << "\n=== 5. GOAL PLANNER ===\n";
    Goal g("Laptop", 80000, 2), h("Bike", 150000, 3);
    for (const Goal& x : {g, h})
        cout << x.describe() << ": save Rs " << setprecision(0)
             << x.requiredMonthly(0.07) << " per month (7% return)\n";

    cout << "\n=== 6. EXCEPTION HANDLING ===\n";
    SavingsAccount sa("Bank Savings", 5000);
    try { sa.withdraw(4500); } catch (const InsufficientFunds& e) { cout << "Blocked: " << e.what() << "\n"; }
    WalletAccount w("Cash Wallet", 300);
    try { w.withdraw(500); } catch (const InsufficientFunds& e) { cout << "Blocked: " << e.what() << "\n"; }
    ft.save("transactions.csv");
    cout << "\nTransactions saved to transactions.csv\n";
    return 0;
}
