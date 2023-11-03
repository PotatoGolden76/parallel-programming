#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <random>
#include <chrono>
#include <string>
#include <shared_mutex>

using namespace std;

int last_transfer = 1;
mutex transfer_mtx;

struct Account {
    int id{};
    int balance{};
    vector<string> log;
    shared_mutex mtx;
};

void transfer(Account &from, Account &to, int amount) {

    if (from.id < to.id) {
        from.mtx.lock();
        to.mtx.lock();
    } else {
        to.mtx.lock();
        from.mtx.lock();
    }
    if (from.balance >= amount) {
        from.balance -= amount;
        transfer_mtx.lock();
        from.log.push_back(
                "Transfer ID: " + to_string(last_transfer) + " | to acc: " + to_string(to.id) + " with amount " +
                to_string(amount));
        to.balance += amount;
        to.log.push_back(
                "Transfer ID: " + to_string(last_transfer) + " | from acc: " + to_string(from.id) + " with amount " +
                to_string(amount));
        last_transfer++;
        transfer_mtx.unlock();
    }

    if (from.id < to.id) {
        from.mtx.unlock();
        to.mtx.unlock();
    } else {
        to.mtx.unlock();
        from.mtx.unlock();
    }
}

void checkConsistency(Account &account, vector<Account> &accounts) {
    int log_bal = 1000, tran_sign;
    bool consistent = false;

    if (account.log.empty()) {
        consistent = true;
    } else {
        for (string &s: account.log) {
            int diff = s.find("with") - s.find("acc:") - 3;
            auto complementary_acc = stoi(s.substr(s.find("acc:") + 4, diff - 1));

            for (auto &cs: accounts[complementary_acc].log) {
                int t = s.find(" | ");
                if (s.substr(0, t) == cs.substr(0, t)) {
//                    cout << endl << s.substr(0, t) << "\t" << cs.substr(0, t) << endl;
                    consistent = true;
                }
            }

            if (s.find("from") == string::npos) {
                tran_sign = -1;
            } else {
                tran_sign = 1;
            }

            int pos = s.find("amount");
            log_bal += tran_sign * (stoi(s.substr(pos + 6)));
        }
    }

    if (log_bal == account.balance && consistent) {
        cout << "Account ID: " << account.id << "\t | \t" << "balance: " << account.balance << "\t | \t" << "CONSISTENT"
             << "\n";
    } else {
        cout << account.id << "\t | \t" << account.balance << "\t | \t" << "INCONSISTENT; LOGGED: " << log_bal << "\n";
    }


}

int main() {
    const int numAccounts = 20;
    const int numThreads = 10;

    vector<Account> accounts(numAccounts);

    // Initialize account balances
    for (int i = 0; i < numAccounts; i++) {
        accounts[i].id = i;
        accounts[i].balance = 1000;
    }

    vector<thread> threads;

    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back([&]() {
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<int> dist(0, numAccounts - 1);
            uniform_int_distribution<int> val_dist(0, 100);

            while (true) {
                int from = dist(gen);
                int to = dist(gen);
                int amount = val_dist(gen);

                if (from != to) {
                    transfer(accounts[from], accounts[to], amount);
                }
                this_thread::sleep_for(chrono::seconds(dist(gen)));
            }
        });
    }


    int numConThreads = 4;
    vector<thread> conThreads;
    for (int id = 0; id < numConThreads; id++) {
        conThreads.emplace_back([&accounts, id]() {
            int tID = id+1;
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<int> dist(2, 7);
            int i = 0;

            while (true) {
                for (auto &account: accounts)
                    account.mtx.lock_shared();

                system("cls");
                cout << "Consistency thread " << tID << ", check number " << ++i << endl;
                for (Account &account: accounts) {
                    checkConsistency(account, accounts);
                }

                for (auto &account: accounts)
                    account.mtx.unlock();
                this_thread::sleep_for(chrono::seconds(dist(gen)));
            }
        });
    }

    for (thread &t: threads) {
        t.join();
    }
    for (thread &t: conThreads) {
        t.join();
    }

    return 0;
}
