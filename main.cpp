#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <limits>
#include <chrono>
#include <ctime>
#include <queue>
#include <unordered_map>
#include <memory>
#include <thread>
#include <algorithm>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

class Date;
ostream& operator<<(ostream& os, const Date& date);
ofstream& operator<<(ofstream& ofs, const Date& date);
ifstream& operator>>(ifstream& ifs, Date& date);

class Date {
public:
    int day, month, year;

    Date() {
        auto now = chrono::system_clock::now();
        time_t now_time = chrono::system_clock::to_time_t(now);
        tm* now_tm = localtime(&now_time);
        
        day = now_tm->tm_mday;
        month = now_tm->tm_mon + 1;
        year = now_tm->tm_year + 1900;
    }
    
    Date(int d, int m, int y) : day(d), month(m), year(y) {}
    
    string toString() const {
        return to_string(day) + "/" + to_string(month) + "/" + to_string(year);
    }
    
    friend ostream& operator<<(ostream& os, const Date& date);
    friend ofstream& operator<<(ofstream& ofs, const Date& date);
    friend ifstream& operator>>(ifstream& ifs, Date& date);
};

ostream& operator<<(ostream& os, const Date& date) {
    os<<date.toString();
    return os;
}

ofstream& operator<<(ofstream& ofs, const Date& date) {
    ofs<<date.day<<" "<<date.month<<" "<<date.year;
    return ofs;
}

ifstream& operator>>(ifstream& ifs, Date& date) {
    ifs>>date.day>>date.month>>date.year;
    return ifs;
}

class Transaction;

struct UpcomingPayment {
    Date dueDate;
    string description;
    double amount;
    bool isInvestment;
    
    UpcomingPayment(const Date& date, const string& desc, double amt, bool inv = false)
        : dueDate(date), description(desc), amount(amt), isInvestment(inv) {}
};

struct PaymentCompare {
    bool operator()(const UpcomingPayment& a, const UpcomingPayment& b) const {
        if (a.dueDate.year != b.dueDate.year) return a.dueDate.year > b.dueDate.year;
        if (a.dueDate.month != b.dueDate.month) return a.dueDate.month > b.dueDate.month;
        return a.dueDate.day > b.dueDate.day;
    }
};

class TrieNode {
public:
    unordered_map<char, unique_ptr<TrieNode>> children;
    bool isEndOfWord;
    
    TrieNode() : isEndOfWord(false) {}
};

class Trie {
private:
    unique_ptr<TrieNode> root;
    
    void getSuggestionsRecursive(TrieNode* node, string& prefix, vector<string>& result) {
        if (node->isEndOfWord) {
            result.push_back(prefix);
        }
        
        for (const auto& pair : node->children) {
            prefix.push_back(pair.first);
            getSuggestionsRecursive(pair.second.get(), prefix, result);
            prefix.pop_back();
        }
    }

public:
    Trie() : root(make_unique<TrieNode>()) {}
    
    void insert(const string& word) {
        TrieNode* current = root.get();
        for (char c : word) {
            if (!current->children[c]) {
                current->children[c] = make_unique<TrieNode>();
            }
            current = current->children[c].get();
        }
        current->isEndOfWord = true;
    }
    
    vector<string> getSuggestions(const string& prefix) {
        vector<string> suggestions;
        TrieNode* current = root.get();
        
        for (char c : prefix) {
            if (!current->children[c]) {
                return suggestions;
            }
            current = current->children[c].get();
        }
        
        string currentPrefix = prefix;
        getSuggestionsRecursive(current, currentPrefix, suggestions);
        return suggestions;
    }
};

class TransactionIndex {
private:
    unordered_map<string, Transaction*> transactionMap;
    int nextId;
    
    string generateId() {
        return "TXN" + to_string(++nextId);
    }

public:
    TransactionIndex() : nextId(0) {}
    
    string addTransaction(Transaction* transaction) {
        string id = generateId();
        transactionMap[id] = transaction;
        return id;
    }
    
    Transaction* getTransaction(const string& id) {
        auto it = transactionMap.find(id);
        return it != transactionMap.end() ? it->second : nullptr;
    }
};

enum class Category {
    INCOME,
    FOOD,
    HOUSING,
    TRANSPORTATION,
    ENTERTAINMENT,
    UTILITIES,
    HEALTHCARE,
    EDUCATION,
    OTHER
};

string categoryToString(Category cat) {
    switch(cat) {
        case Category::INCOME: return "Income";
        case Category::FOOD: return "Food";
        case Category::HOUSING: return "Housing";
        case Category::TRANSPORTATION: return "Transportation";
        case Category::ENTERTAINMENT: return "Entertainment";
        case Category::UTILITIES: return "Utilities";
        case Category::HEALTHCARE: return "Healthcare";
        case Category::EDUCATION: return "Education";
        case Category::OTHER: return "Other";
        default: return "Unknown";
    }
}

Category stringToCategory(const string& str) {
    if (str == "Income") return Category::INCOME;
    if (str == "Food") return Category::FOOD;
    if (str == "Housing") return Category::HOUSING;
    if (str == "Transportation") return Category::TRANSPORTATION;
    if (str == "Entertainment") return Category::ENTERTAINMENT;
    if (str == "Utilities") return Category::UTILITIES;
    if (str == "Healthcare") return Category::HEALTHCARE;
    if (str == "Education") return Category::EDUCATION;
    return Category::OTHER;
}

class Transaction {
protected:
    double amount;
    string description;
    Date date;
    Category category;

public:
    Transaction(double amt, const string &des, Category cat = Category::OTHER) {
        amount = amt;
        description = des;
        date = Date();
        category = cat;
    }
    
    Transaction(double amt, const string &des, const Date& dt, Category cat = Category::OTHER) {
        amount = amt;
        description = des;
        date = dt;
        category = cat;
    }

    virtual void display() {
        cout<<setw(12)<<date<<setw(15)<<amount<<setw(15)<<categoryToString(category)<<setw(20)<<description;
    }
    
    virtual double getAmount() const {
        return amount;
    }
    
    Category getCategory() const {
        return category;
    }
    
    Date getDate() const {
        return date;
    }
    
    string getDescription() const {
        return description;
    }
    
    virtual void saveToFile(ofstream& file) const {
        file<<"T "<<amount<<" "<<description<<" "<<date<<" "<<categoryToString(category)<<endl;
    }
    
    virtual string getType() const {
        return "Transaction";
    }
};

class Income : public Transaction {
public:
    Income(double amt, const string& des, Category cat = Category::INCOME) 
        : Transaction(amt, des, cat) {}
    
    Income(double amt, const string& des, const Date& dt, Category cat = Category::INCOME) 
        : Transaction(amt, des, dt, cat) {}

    void display() override {
        cout<<setw(15)<<"Income";
        Transaction::display();
        cout<<endl;
    }
    
    void saveToFile(ofstream& file) const override {
        file<<"I "<<amount<<" "<<description<<" "<<date<<" "<<categoryToString(category)<<endl;
    }
    
    string getType() const override {
        return "Income";
    }
};

class Expenditure : public Transaction {
public:
    Expenditure(double amt, const string &des, Category cat = Category::OTHER) 
        : Transaction(amt, des, cat) {}
    
    Expenditure(double amt, const string &des, const Date& dt, Category cat = Category::OTHER) 
        : Transaction(amt, des, dt, cat) {}

    void display() override {
        cout<<setw(15)<<"Expenditure";
        Transaction::display();
        cout<<endl;
    }
    
    void saveToFile(ofstream& file) const override {
        file<<"E "<<amount<<" "<<description<<" "<<date<<" "<<categoryToString(category)<<endl;
    }
    
    string getType() const override {
        return "Expenditure";
    }
};

class Investment {
protected:
    double amount;
    int duration;
    Date startDate;

public:
    Investment(double amt, int dur) {
        amount = amt;
        duration = dur;
        startDate = Date();
    }
    
    Investment(double amt, int dur, const Date& dt) {
        amount = amt;
        duration = dur;
        startDate = dt;
    }

    virtual void display() {
        cout<<setw(15)<<amount<<setw(15)<<duration<<setw(15)<<startDate;
    }

    virtual double maturityAmount() {
        return amount;
    }
    
    double getAmount() const {
        return amount;
    }
    
    int getDuration() const {
        return duration;
    }
    
    Date getStartDate() const {
        return startDate;
    }
    
    virtual void saveToFile(ofstream& file) const {
        file<<"INV "<<amount<<" "<<duration<<" "<<startDate<<endl;
    }
    
    virtual string getType() const {
        return "Investment";
    }
};

class SIP : public Investment {
private:
    double monthly;

public:
    SIP(double amt, int dur, double monAmt) : Investment(amt, dur) {
        monthly = monAmt;
    }
    
    SIP(double amt, int dur, double monAmt, const Date& dt) : Investment(amt, dur, dt) {
        monthly = monAmt;
    }

    void display() override {
        cout<<setw(15)<<"SIP";
        Investment::display();
        cout<<setw(20)<<monthly<<endl;
    }

    double maturityAmount() override {
        double final = amount * pow(1 + (0.096/12), duration*12);
        return final + (monthly * 12 * duration);
    }
    
    double getMonthly() const {
        return monthly;
    }
    
    void saveToFile(ofstream& file) const override {
        file<<"SIP "<<amount<<" "<<duration<<" "<<startDate<<" "<<monthly<<endl;
    }
    
    string getType() const override {
        return "SIP";
    }
};

class FD : public Investment {
public:
    FD(double amt, int dur) : Investment(amt, dur) {}
    
    FD(double amt, int dur, const Date& dt) : Investment(amt, dur, dt) {}

    void display() override {
        cout<<setw(15)<<"FD";
        Investment::display();
        cout<<endl;
    }

    double maturityAmount() override {
        return amount * pow((1 + 0.071), duration);
    }
    
    void saveToFile(ofstream& file) const override {
        file<<"FD "<<amount<<" "<<duration<<" "<<startDate<<endl;
    }
    
    string getType() const override {
        return "FD";
    }
};

class FinanceManager {
private:
    priority_queue<UpcomingPayment, vector<UpcomingPayment>, PaymentCompare> upcomingPayments;
    Trie descriptionTrie;
    TransactionIndex transactionIndex;

public:
    vector<Transaction*> transactions;
    vector<Investment*> investments;

    FinanceManager() {}
    
    ~FinanceManager() {
        for (auto t : transactions) {
            delete t;
        }
        for (auto i : investments) {
            delete i;
        }
    }

    void addTransaction(Transaction* t) {
        transactions.push_back(t);
        descriptionTrie.insert(t->getDescription());
        string id = transactionIndex.addTransaction(t);
    }

    void addInvestment(Investment* i) {
        investments.push_back(i);
    }
    
    // search transactions by description (this is just linear search)
    vector<Transaction*> searchTransactionsByDescription(const string& description) {
        vector<Transaction*> results;
        for (auto t : transactions) {
            if (t->getDescription().find(description) != string::npos) {
                results.push_back(t);
            }
        }
        return results;
    }
    
    // search transactions(by date)
    vector<Transaction*> searchTransactionsByDate(const Date& date) {
        vector<Transaction*> results;
        for (auto t : transactions) {
            Date tDate = t->getDate();
            if (tDate.day == date.day && tDate.month == date.month && tDate.year == date.year) {
                results.push_back(t);
            }
        }
        return results;
    }
    
    // search transactions(by category)
    vector<Transaction*> searchTransactionsByCategory(Category category) {
        vector<Transaction*> results;
        for (auto t : transactions) {
            if (t->getCategory() == category) {
                results.push_back(t);
            }
        }
        return results;
    }
    
    // search investments(by amount range)
    vector<Investment*> searchInvestmentsByAmountRange(double minAmount, double maxAmount) {
        vector<Investment*> results;
        for (auto i : investments) {
            double amount = i->getAmount();
            if (amount >= minAmount && amount <= maxAmount) {
                results.push_back(i);
            }
        }
        return results;
    }
    
    // search investments- type
    vector<Investment*> searchInvestmentsByType(const string& type) {
        vector<Investment*> results;
        for (auto i : investments) {
            if (i->getType() == type) {
                results.push_back(i);
            }
        }
        return results;
    }
    
    void displayTransactionSearchResults(const vector<Transaction*>& results) {
        if (results.empty()) {
            cout << "No matching transactions found." << endl;
            return;
        }
        
        cout << "\n--SEARCH RESULTS--" << endl;
        cout << setw(15) << "Type" << setw(12) << "Date" << setw(15) << "Amount" 
             << setw(15) << "Category" << setw(20) << "Description" << endl;
        cout << string(77, '-') << endl;
        
        for (auto t : results) {
            t->display();
        }
    }
    
    void displayInvestmentSearchResults(const vector<Investment*>& results) {
        if (results.empty()) {
            cout << "No matching investments found." << endl;
            return;
        }
        
        cout << "\n--SEARCH RESULTS--" << endl;
        cout << setw(15) << "Type" << setw(15) << "Amount" << setw(15) << "Duration" 
             << setw(15) << "Start Date" << setw(20) << "Monthly amount" << endl;
        cout << string(80, '-') << endl;
        
        for (auto i : results) {
            i->display();
        }
    }
    
    bool deleteTransaction(int index) {
        if (index < 0 || index >= transactions.size()) {
            return false;
        }
        
        delete transactions[index];
        transactions.erase(transactions.begin() + index);
        return true;
    }
    
    bool deleteInvestment(int index) {
        if (index < 0 || index >= investments.size()) {
            return false;
        }
        
        delete investments[index];
        investments.erase(investments.begin() + index);
        return true;
    }
    
    bool updateTransaction(int index, Transaction* newTransaction) {
        if (index < 0 || index >= transactions.size()) {
            return false;
        }
        
        delete transactions[index];
        transactions[index] = newTransaction;
        return true;
    }
    
    bool updateInvestment(int index, Investment* newInvestment) {
        if (index < 0 || index >= investments.size()) {
            return false;
        }
        
        delete investments[index];
        investments[index] = newInvestment;
        return true;
    }
    
    void sortTransactionsByAmount(bool ascending = true) {
        if (ascending) {
            sort(transactions.begin(), transactions.end(), 
                 [](Transaction* a, Transaction* b) { return a->getAmount() < b->getAmount(); });
        } else {
            sort(transactions.begin(), transactions.end(), 
                 [](Transaction* a, Transaction* b) { return a->getAmount() > b->getAmount(); });
        }
    }
    
    void sortTransactionsByDate(bool ascending = true) {
        if (ascending) {
            sort(transactions.begin(), transactions.end(), 
                 [](Transaction* a, Transaction* b) {
                     Date dateA = a->getDate();
                     Date dateB = b->getDate();
                     if (dateA.year != dateB.year) return dateA.year < dateB.year;
                     if (dateA.month != dateB.month) return dateA.month < dateB.month;
                     return dateA.day < dateB.day;
                 });
        } else {
            sort(transactions.begin(), transactions.end(), 
                 [](Transaction* a, Transaction* b) {
                     Date dateA = a->getDate();
                     Date dateB = b->getDate();
                     if (dateA.year != dateB.year) return dateA.year > dateB.year;
                     if (dateA.month != dateB.month) return dateA.month > dateB.month;
                     return dateA.day > dateB.day;
                 });
        }
    }
    
    void sortTransactionsByCategory() {
        sort(transactions.begin(), transactions.end(), 
             [](Transaction* a, Transaction* b) {
                 return static_cast<int>(a->getCategory()) < static_cast<int>(b->getCategory());
             });
    }
    
    void sortInvestmentsByAmount(bool ascending = true) {
        if (ascending) {
            sort(investments.begin(), investments.end(), 
                 [](Investment* a, Investment* b) { return a->getAmount() < b->getAmount(); });
        } else {
            sort(investments.begin(), investments.end(), 
                 [](Investment* a, Investment* b) { return a->getAmount() > b->getAmount(); });
        }
    }
    
    void sortInvestmentsByDuration(bool ascending = true) {
        if (ascending) {
            sort(investments.begin(), investments.end(), 
                 [](Investment* a, Investment* b) { return a->getDuration() < b->getDuration(); });
        } else {
            sort(investments.begin(), investments.end(), 
                 [](Investment* a, Investment* b) { return a->getDuration() > b->getDuration(); });
        }
    }

    void addUpcomingPayment(const Date& date, const string& desc, double amount, bool isInvestment = false) {
        upcomingPayments.push(UpcomingPayment(date, desc, amount, isInvestment));
    }
    
    void displayUpcomingPayments() {
        auto tempQueue = upcomingPayments;
        cout<<"\n--UPCOMING PAYMENTS--"<<endl;
        cout<<setw(12)<<"Date"<<setw(20)<<"Description"<<setw(15)<<"Amount"<<setw(15)<<"Type"<<endl;
        cout<<string(62, '-')<<endl;
        
        while (!tempQueue.empty()) {
            const auto& payment = tempQueue.top();
            cout<<setw(12)<<payment.dueDate 
                <<setw(20)<<payment.description 
                <<setw(15)<<fixed<<setprecision(2)<<payment.amount
                <<setw(15)<<(payment.isInvestment ? "Investment" : "Payment")<<endl;
            tempQueue.pop();
        }
    }
    
    vector<string> getDescriptionSuggestions(const string& prefix) {
        return descriptionTrie.getSuggestions(prefix);
    }

    void displayRecord(double balance) {
        cout<<"-----------------------------------"<<endl;
        cout<<"|        Personal Finance        |"<<endl;
        cout<<"-----------------------------------"<<endl;

        cout<<"\n||--BALANCE--: "<<fixed<<setprecision(2)<<balance<<"||"<<endl;

        cout<<"\n--SAVINGS--: "<<endl;
        cout<<setw(15)<<"Type"<<setw(12)<<"Date"<<setw(15)<<"Amount"<<setw(15)<<"Category"<<setw(20)<<"Description"<<endl;
        cout<<string(77, '-')<<endl;
        for (auto t : transactions) {
            t->display();
        }

        cout<<"\n--INVESTMENTS--"<<endl;
        cout<<setw(15)<<"Type"<<setw(15)<<"Amount"<<setw(15)<<"Duration"<<setw(15)<<"Start Date"<<setw(20)<<"Monthly amount"<<endl;
        cout<<string(80, '-')<<endl;
        for (auto i : investments) {
            i->display();
        }
    }
    
    void generateMonthlyReport(int month, int year) {
        cout<<"\n----- Monthly Report for "<<month<<"/"<<year<<" -----"<<endl;
        
        double totalIncome = 0.0;
        double totalExpense = 0.0;
        map<Category, double> categoryExpenses;
        
        for (auto t : transactions) {
            Date date = t->getDate();
            if (date.toString().find("/" + to_string(month) + "/" + to_string(year)) != string::npos) {
                if (t->getType() == "Income") {
                    totalIncome += t->getAmount();
                } else if (t->getType() == "Expenditure") {
                    totalExpense += t->getAmount();
                    categoryExpenses[t->getCategory()] += t->getAmount();
                }
            }
        }
        
        cout<<"Total Income: "<<fixed<<setprecision(2)<<totalIncome<<endl;
        cout<<"Total Expenses: "<<fixed<<setprecision(2)<<totalExpense<<endl;
        cout<<"Net Savings: "<<fixed<<setprecision(2)<<(totalIncome - totalExpense)<<endl;
        
        cout<<"\nExpense Breakdown by Category:"<<endl;
        for (const auto& pair : categoryExpenses) {
            cout<<setw(20)<<categoryToString(pair.first)<<": "<<fixed<<setprecision(2)<<pair.second;
            if (totalExpense > 0) {
                cout<<" ("<<fixed<<setprecision(1)<<(pair.second / totalExpense * 100)<<"%)";
            }
            cout<<endl;
        }
    }
    
    bool saveToFile(const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        file<<transactions.size()<<endl;
        for (auto t : transactions) {
            t->saveToFile(file);
        }
        
        file<<investments.size()<<endl;
        for (auto i : investments) {
            i->saveToFile(file);
        }
        
        file.close();
        return true;
    }
    
    bool loadFromFile(const string& filename, double& balance) {
        ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        for (auto t : transactions) delete t;
        for (auto i : investments) delete i;
        transactions.clear();
        investments.clear();
        
        int transactionCount;
        file>>transactionCount;
        file.ignore();
        
        for (int i = 0; i < transactionCount; i++) {
            char type;
            double amount;
            string description, categoryStr;
            int day, month, year;
            
            file>>type>>amount;
            file.ignore();
            
            getline(file, description, ' ');
            
            file>>day>>month>>year;
            Date date(day, month, year);
            
            file>>categoryStr;
            
            Category category = stringToCategory(categoryStr);
            
            if (type == 'I') {
                transactions.push_back(new Income(amount, description, date, category));
                balance += amount;
            } else if (type == 'E') {
                transactions.push_back(new Expenditure(amount, description, date, category));
                balance -= amount;
            }
        }
        
        int investmentCount;
        file>>investmentCount;
        file.ignore();
        
        for (int i = 0; i < investmentCount; i++) {
            string type;
            double amount;
            int duration;
            int day, month, year;
            
            file>>type>>amount>>duration;
            file>>day>>month>>year;
            Date startDate(day, month, year);
            
            if (type == "SIP") {
                double monthly;
                file>>monthly;
                investments.push_back(new SIP(amount, duration, monthly, startDate));
            } else if (type == "FD") {
                investments.push_back(new FD(amount, duration, startDate));
            }
            
            balance -= amount;
        }
        
        file.close();
        return true;
    }
};

class User {
public:
    FinanceManager manager;
    double balance;
    string username;
    string dataFile;

    User(double initialBalance, const string& name = "default") {
        balance = initialBalance;
        username = name;
        dataFile = username + "_finance_data.txt";
        
        if (!manager.loadFromFile(dataFile, balance)) {
            cout<<"No existing data found. Starting with a fresh account."<<endl;
            cout<<endl<<endl;
            system("pause");
        } else {
            cout<<"Loaded existing data for "<<username<<"."<<endl;
            cout<<endl<<endl;
            system("pause");
        }
    }
    
    ~User() {
        saveData();
    }
    
    bool saveData() {
        return manager.saveToFile(dataFile);
    }

    void searchTransactions() {
        int searchOption;
        cout<<"\n--SEARCH TRANSACTIONS--"<<endl;
        cout<<"1. Search by Description"<<endl;
        cout<<"2. Search by Date"<<endl;
        cout<<"3. Search by Category"<<endl;
        cout<<"Enter choice: ";
        cin>>searchOption;
        
        vector<Transaction*> results;
        
        switch(searchOption) {
            case 1: {
                string description;
                cout<<"Enter description to search for: ";
                cin.ignore();
                getline(cin, description);
                results = manager.searchTransactionsByDescription(description);
                break;
            }
            case 2: {
                int day, month, year;
                cout<<"Enter date (day month year): ";
                cin>>day>>month>>year;
                Date searchDate(day, month, year);
                results = manager.searchTransactionsByDate(searchDate);
                break;
            }
            case 3: {
                int categoryOption;
                cout<<"Select category:"<<endl;
                cout<<"1. Income"<<endl;
                cout<<"2. Food"<<endl;
                cout<<"3. Housing"<<endl;
                cout<<"4. Transportation"<<endl;
                cout<<"5. Entertainment"<<endl;
                cout<<"6. Utilities"<<endl;
                cout<<"7. Healthcare"<<endl;
                cout<<"8. Education"<<endl;
                cout<<"9. Other"<<endl;
                cout<<"Enter choice: ";
                cin>>categoryOption;
                
                Category category;
                switch(categoryOption) {
                    case 1: category = Category::INCOME; break;
                    case 2: category = Category::FOOD; break;
                    case 3: category = Category::HOUSING; break;
                    case 4: category = Category::TRANSPORTATION; break;
                    case 5: category = Category::ENTERTAINMENT; break;
                    case 6: category = Category::UTILITIES; break;
                    case 7: category = Category::HEALTHCARE; break;
                    case 8: category = Category::EDUCATION; break;
                    default: category = Category::OTHER; break;
                }
                
                results = manager.searchTransactionsByCategory(category);
                break;
            }
            default:
                cout<<"Invalid option!"<<endl;
                return;
        }
        
        manager.displayTransactionSearchResults(results);
    }
    
    void searchInvestments() {
        int searchOption;
        cout<<"\n--SEARCH INVESTMENTS--"<<endl;
        cout<<"1. Search by Amount Range"<<endl;
        cout<<"2. Search by Type (FD/SIP)"<<endl;
        cout<<"Enter choice: ";
        cin>>searchOption;
        
        vector<Investment*> results;
        
        switch(searchOption) {
            case 1: {
                double minAmount, maxAmount;
                cout<<"Enter minimum amount: ";
                cin>>minAmount;
                cout<<"Enter maximum amount: ";
                cin>>maxAmount;
                results = manager.searchInvestmentsByAmountRange(minAmount, maxAmount);
                break;
            }
            case 2: {
                int typeOption;
                cout<<"Select investment type:"<<endl;
                cout<<"1. Fixed Deposit (FD)"<<endl;
                cout<<"2. Systematic Investment Plan (SIP)"<<endl;
                cout<<"Enter choice: ";
                cin>>typeOption;
                
                string type = (typeOption == 1) ? "FD" : "SIP";
                results = manager.searchInvestmentsByType(type);
                break;
            }
            default:
                cout<<"Invalid option!"<<endl;
                return;
        }
        
        manager.displayInvestmentSearchResults(results);
    }
    
    void deleteRecord() {
        int deleteOption;
        cout<<"\n--DELETE RECORD--"<<endl;
        cout<<"1. Delete Transaction"<<endl;
        cout<<"2. Delete Investment"<<endl;
        cout<<"Enter choice: ";
        cin>>deleteOption;
        
        switch(deleteOption) {
            case 1: {
                if (manager.transactions.empty()) {
                    cout<<"No transactions to delete!"<<endl;
                    return;
                }
                
                cout<<"\n--TRANSACTIONS--"<<endl;
                cout<<setw(5)<<"Index"<<setw(15)<<"Type"<<setw(12)<<"Date"<<setw(15)<<"Amount"
                    <<setw(15)<<"Category"<<setw(20)<<"Description"<<endl;
                cout<<string(82, '-')<<endl;
                
                for (size_t i = 0; i < manager.transactions.size(); i++) {
                    cout<<setw(5)<<i;
                    manager.transactions[i]->display();
                }
                
                int index;
                cout<<"\nEnter index of transaction to delete: ";
                cin>>index;
                
                if (manager.deleteTransaction(index)) {
                    cout<<"Transaction deleted successfully!"<<endl;
                } else {
                    cout<<"Invalid index!"<<endl;
                }
                break;
            }
            case 2: {
                if (manager.investments.empty()) {
                    cout<<"No investments to delete!"<<endl;
                    return;
                }
                
                cout<<"\n--INVESTMENTS--"<<endl;
                cout<<setw(5)<<"Index"<<setw(15)<<"Type"<<setw(15)<<"Amount"<<setw(15)<<"Duration"
                    <<setw(15)<<"Start Date"<<setw(20)<<"Monthly amount"<<endl;
                cout<<string(85, '-')<<endl;
                
                for (size_t i = 0; i < manager.investments.size(); i++) {
                    cout<<setw(5)<<i;
                    manager.investments[i]->display();
                }
                
                int index;
                cout<<"\nEnter index of investment to delete: ";
                cin>>index;
                
                if (manager.deleteInvestment(index)) {
                    cout<<"Investment deleted successfully!"<<endl;
                } else {
                    cout<<"Invalid index!"<<endl;
                }
                break;
            }
            default:
                cout<<"Invalid option!"<<endl;
                return;
        }
    }
    
    void updateRecord() {
        int updateOption;
        cout<<"\n--UPDATE RECORD--"<<endl;
        cout<<"1. Update Transaction"<<endl;
        cout<<"2. Update Investment"<<endl;
        cout<<"Enter choice: ";
        cin>>updateOption;
        
        switch(updateOption) {
            case 1: {
                if (manager.transactions.empty()) {
                    cout<<"No transactions to update!"<<endl;
                    return;
                }
                
                cout<<"\n--TRANSACTIONS--"<<endl;
                cout<<setw(5)<<"Index"<<setw(15)<<"Type"<<setw(12)<<"Date"<<setw(15)<<"Amount"
                    <<setw(15)<<"Category"<<setw(20)<<"Description"<<endl;
                cout<<string(82, '-')<<endl;
                
                for (size_t i = 0; i < manager.transactions.size(); i++) {
                    cout<<setw(5)<<i;
                    manager.transactions[i]->display();
                }
                
                int index;
                cout<<"\nEnter index of transaction to update: ";
                cin>>index;
                
                if (index < 0 || index >= manager.transactions.size()) {
                    cout<<"Invalid index!"<<endl;
                    return;
                }
                
                Transaction* oldTransaction = manager.transactions[index];
                string oldType = oldTransaction->getType();
                double oldAmount = oldTransaction->getAmount();
                
                int typeOption;
                cout<<"\nSelect new transaction type:"<<endl;
                cout<<"1. Income"<<endl;
                cout<<"2. Expenditure"<<endl;
                cout<<"Enter choice: ";
                cin>>typeOption;
                
                double amount;
                string description;
                int day, month, year;
                int categoryOption;
                
                cout<<"Enter new amount: ";
                cin>>amount;
                cout<<"Enter new description: ";
                cin.ignore();
                getline(cin, description);
                cout<<"Enter new date (day month year): ";
                cin>>day>>month>>year;
                
                cout<<"Select new category:"<<endl;
                if (typeOption == 1) {
                    cout<<"1. Income"<<endl;
                    categoryOption = 1;
                } else {
                    cout<<"1. Food"<<endl;
                    cout<<"2. Housing"<<endl;
                    cout<<"3. Transportation"<<endl;
                    cout<<"4. Entertainment"<<endl;
                    cout<<"5. Utilities"<<endl;
                    cout<<"6. Healthcare"<<endl;
                    cout<<"7. Education"<<endl;
                    cout<<"8. Other"<<endl;
                    cout<<"Enter choice: ";
                    cin>>categoryOption;
                }
                
                Category category;
                if (typeOption == 1) {
                    category = Category::INCOME;
                } else {
                    switch(categoryOption) {
                        case 1: category = Category::FOOD; break;
                        case 2: category = Category::HOUSING; break;
                        case 3: category = Category::TRANSPORTATION; break;
                        case 4: category = Category::ENTERTAINMENT; break;
                        case 5: category = Category::UTILITIES; break;
                        case 6: category = Category::HEALTHCARE; break;
                        case 7: category = Category::EDUCATION; break;
                        default: category = Category::OTHER; break;
                    }
                }
                
                Date date(day, month, year);
                Transaction* newTransaction;
                
                if (typeOption == 1) {
                    newTransaction = new Income(amount, description, date, category);
                } else {
                    newTransaction = new Expenditure(amount, description, date, category);
                }
                
                if (oldType == "Income") {
                    balance -= oldAmount;
                } else {
                    balance += oldAmount;
                }
                
                if (typeOption == 1) {
                    balance += amount;
                } else {
                    balance -= amount;
                }
                
                if (manager.updateTransaction(index, newTransaction)) {
                    cout<<"Transaction updated successfully!"<<endl;
                } else {
                    cout<<"Update failed!"<<endl;
                }
                break;
            }
            case 2: {
                if (manager.investments.empty()) {
                    cout<<"No investments to update!"<<endl;
                    return;
                }
                
                cout<<"\n--INVESTMENTS--"<<endl;
                cout<<setw(5)<<"Index"<<setw(15)<<"Type"<<setw(15)<<"Amount"<<setw(15)<<"Duration"
                    <<setw(15)<<"Start Date"<<setw(20)<<"Monthly amount"<<endl;
                cout<<string(85, '-')<<endl;
                
                for (size_t i = 0; i < manager.investments.size(); i++) {
                    cout<<setw(5)<<i;
                    manager.investments[i]->display();
                }
                
                int index;
                cout<<"\nEnter index of investment to update: ";
                cin>>index;
                
                if (index < 0 || index >= manager.investments.size()) {
                    cout<<"Invalid index!"<<endl;
                    return;
                }
                
                Investment* oldInvestment = manager.investments[index];
                double oldAmount = oldInvestment->getAmount();
                
                int typeOption;
                cout<<"\nSelect new investment type:"<<endl;
                cout<<"1. Fixed Deposit (FD)"<<endl;
                cout<<"2. Systematic Investment Plan (SIP)"<<endl;
                cout<<"Enter choice: ";
                cin>>typeOption;
                
                double amount;
                int duration;
                int day, month, year;
                
                cout<<"Enter new amount: ";
                cin>>amount;
                cout<<"Enter new duration (in years): ";
                cin>>duration;
                cout<<"Enter new start date (day month year): ";
                cin>>day>>month>>year;
                
                Date startDate(day, month, year);
                Investment* newInvestment;
                
                if (typeOption == 1) {
                    newInvestment = new FD(amount, duration, startDate);
                } else {
                    double monthly;
                    cout<<"Enter new monthly investment amount: ";
                    cin>>monthly;
                    newInvestment = new SIP(amount, duration, monthly, startDate);
                }
                
                balance += oldAmount;
                balance -= amount;
                
                if (manager.updateInvestment(index, newInvestment)) {
                    cout<<"Investment updated successfully!"<<endl;
                } else {
                    cout<<"Update failed!"<<endl;
                }
                break;
            }
            default:
                cout<<"Invalid option!"<<endl;
                return;
        }
    }
    
    void sortRecords() {
        int sortOption;
        cout<<"\n--SORT RECORDS--"<<endl;
        cout<<"1. Sort Transactions"<<endl;
        cout<<"2. Sort Investments"<<endl;
        cout<<"Enter choice: ";
        cin>>sortOption;
        
        switch(sortOption) {
            case 1: {
                int field;
                cout<<"\nSort transactions by:"<<endl;
                cout<<"1. Amount (Ascending)"<<endl;
                cout<<"2. Amount (Descending)"<<endl;
                cout<<"3. Date (Newest First)"<<endl;
                cout<<"4. Date (Oldest First)"<<endl;
                cout<<"5. Category"<<endl;
                cout<<"Enter choice: ";
                cin>>field;
                
                switch(field) {
                    case 1:
                        manager.sortTransactionsByAmount(true);
                        break;
                    case 2:
                        manager.sortTransactionsByAmount(false);
                        break;
                    case 3:
                        manager.sortTransactionsByDate(false);
                        break;
                    case 4:
                        manager.sortTransactionsByDate(true);
                        break;
                    case 5:
                        manager.sortTransactionsByCategory();
                        break;
                    default:
                        cout<<"Invalid option!"<<endl;
                        return;
                }
                
                cout<<"\nTransactions sorted successfully!"<<endl;
                cout<<"\nSorted Transactions:"<<endl;
                cout<<setw(15)<<"Type"<<setw(12)<<"Date"<<setw(15)<<"Amount"
                    <<setw(15)<<"Category"<<setw(20)<<"Description"<<endl;
                cout<<string(77, '-')<<endl;
                
                for (auto t : manager.transactions) {
                    t->display();
                }
                break;
            }
            case 2: {
                int field;
                cout<<"\nSort investments by:"<<endl;
                cout<<"1. Amount (Ascending)"<<endl;
                cout<<"2. Amount (Descending)"<<endl;
                cout<<"3. Duration (Ascending)"<<endl;
                cout<<"4. Duration (Descending)"<<endl;
                cout<<"Enter choice: ";
                cin>>field;
                
                switch(field) {
                    case 1:
                        manager.sortInvestmentsByAmount(true);
                        break;
                    case 2:
                        manager.sortInvestmentsByAmount(false);
                        break;
                    case 3:
                        manager.sortInvestmentsByDuration(true);
                        break;
                    case 4:
                        manager.sortInvestmentsByDuration(false);
                        break;
                    default:
                        cout<<"Invalid option!"<<endl;
                        return;
                }
                
                cout<<"\nInvestments sorted successfully!"<<endl;
                cout<<"\nSorted Investments:"<<endl;
                cout<<setw(15)<<"Type"<<setw(15)<<"Amount"<<setw(15)<<"Duration"
                    <<setw(15)<<"Start Date"<<setw(20)<<"Monthly amount"<<endl;
                cout<<string(80, '-')<<endl;
                
                for (auto i : manager.investments) {
                    i->display();
                }
                break;
            }
            default:
                cout<<"Invalid option!"<<endl;
                return;
        }
    }

    void operations() {
        int choice = -1;
        while (choice != 0) {
            system("cls");
            cout<<"\n--CHOOSE--"<<endl;
            cout<<"1. Record Income"<<endl;
            cout<<"2. Record Expenditure"<<endl;
            cout<<"3. Make Investment"<<endl;
            cout<<"4. Finance Information"<<endl;
            cout<<"5. Investment Information"<<endl;
            cout<<"6. Monthly Report"<<endl;
            cout<<"7. Save Data"<<endl;
            cout<<"8. Add upcoming payment"<<endl;
            cout<<"9. Team Members"<<endl;
            cout<<"10. Search Transactions"<<endl;
            cout<<"11. Search Investments"<<endl;
            cout<<"12. Delete Record"<<endl;
            cout<<"13. Update Record"<<endl;
            cout<<"14. Sort Records"<<endl;
            cout<<"0. Exit"<<endl<<endl<<endl;
            cout<<"Enter choice : ";
            
            cin>>choice;
            
            switch (choice) {
                case 1: {
                    double amount;
                    string description;
                    cout<<"Enter amount: ";
                    cin>>amount;
                    cout<<"Enter description: ";
                    cin.ignore();
                    getline(cin, description);
                    
                    if (amount > 0) {
                        manager.addTransaction(new Income(amount, description));
                        balance += amount;
                        cout<<"Income recorded successfully!"<<endl;
                    } else {
                        cout<<"Invalid amount!"<<endl;
                    }
                        break;
                    }
                case 2: {
                    double amount;
                    string description;
                    cout<<"Enter amount: ";
                    cin>>amount;
                    cout<<"Enter description: ";
                    cin.ignore();
                    getline(cin, description);
                    
                    if (amount > 0 && amount <= balance) {
                        manager.addTransaction(new Expenditure(amount, description));
                        balance -= amount;
                        cout<<"Expenditure recorded successfully!"<<endl;
                    } else {
                        cout<<"Invalid amount or insufficient balance!"<<endl;
                    }
                    break;
                }
                case 3:
                    makeInvestment();
                    break;
                case 4:
                    manager.displayRecord(balance);
                    break;
                case 5:
                    manager.displayRecord(balance);
                    break;
                case 6: {
                    int month, year;
                    cout<<"Enter month (1-12): ";
                    cin>>month;
                    cout<<"Enter year: ";
                    cin>>year;
                    manager.generateMonthlyReport(month, year);
                    break;
                }
                case 7:
                    if (saveData()) {
                        cout<<"Data saved successfully!"<<endl;
                    } else {
                        cout<<"Error saving data!"<<endl;
                    }
                    break;
                case 8: {
                    int day, month, year;
                    double amount;
                    string description;
                    cout<<"Enter due date (day month year): ";
                    cin>>day>>month>>year;
                    cout<<"Enter amount: ";
                    cin>>amount;
                    cout<<"Enter description: ";
                    cin.ignore();
                    getline(cin, description);
                            
                    Date dueDate(day, month, year);
                    manager.addUpcomingPayment(dueDate, description, amount);
                    cout<<"Upcoming payment added successfully!"<<endl;
                    break;
                }
                case 9: {
                    system("cls");
                    system("color 0A");
                    cout<<"\n===> Team Members <===\n"<<endl;
                    
                    vector<string> members = {
                        "Ananya Addisu - BDU1600957",
                        "Abiyu Zewdu - BDU1600628",
                        "Amanuel Amare - BDU1600905",
                        "Abel Amare - BDU1601012",
                        "Nuhamin Enbakum - BDU1602288"
                    };
                    
                    for (const auto& member : members) {
                        for (char c : member) {
                            cout<<c<<flush;
                            sleep_for(milliseconds(50));
                        }
                        cout<<endl;
                        sleep_for(milliseconds(200));
                    }
                    
                    cout<<"\n=== Submitted To ==="<<endl;
                    cout<<"Mr. Jemal"<<endl;
                    cout<<"Department of Software Engineering"<<endl;
                    cout<<"Bahir Dar Institute of Technology\n"<<endl;
                    system("pause");
                    break;
                }
                case 10: {
                    searchTransactions();
                    break;
                }
                case 11: {
                    searchInvestments();
                    break;
                }
                case 12: {
                    deleteRecord();
                    break;
                }
                case 13: {
                    updateRecord();
                    break;
                }
                case 14: {
                    sortRecords();
                    break;
                }
                case 0:
                    cout<<"Exiting..."<<endl;
                    break;
                default:
                    cout<<"Invalid choice!"<<endl;
            }
            
            if (choice != 0) {
                if(choice != 9){
                cout<<"\nPress Enter to continue...";
                system("color 07"); 
                cin.ignore();
                cin.get();
                } else{
                    system("color 07"); 
                }
            }
        }
    }

    void makeInvestment() {
        int choice;
        double amount;
        int duration;
        
        cout<<"\n--INVESTMENT OPTIONS--"<<endl;
        cout<<"1. Fixed Deposit (FD)"<<endl;
        cout<<"2. Systematic Investment Plan (SIP)"<<endl;
        cout<<"Enter choice: ";
        cin>>choice;
        
        cout<<"Enter amount: ";
        cin>>amount;
        cout<<"Enter duration (in years): ";
        cin>>duration;
        
        if (amount <= balance) {
            if (choice == 1) {
                manager.addInvestment(new FD(amount, duration));
                balance -= amount;
                cout<<"FD created successfully!"<<endl;
            } else if (choice == 2) {
                double monthly;
                cout<<"Enter monthly investment amount: ";
                cin>>monthly;
                manager.addInvestment(new SIP(amount, duration, monthly));
                balance -= amount;
                cout<<"SIP created successfully!"<<endl;
            } else {
                cout<<"Invalid choice!"<<endl;
            }
        } else {
            cout<<"Insufficient balance!"<<endl;
        }
    }
};

int main() {
    cout<<"---Welcome to Finance Management System!!---\n"<<endl;
    
    string username;
    cout<<"Enter your username: ";
    cin>>username; 
    
    if (username.empty()) {
        username = "default";
    }
    
    User user(2000, username);
    user.operations();

    return 0;
}
