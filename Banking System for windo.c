// Banking System for Windows
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>

#define MAX_ACCOUNTS 100
#define DATA_FILE "accounts.dat"
#define LOG_FILE "transactions.log"
#define ADMIN_PASSWORD "admin123"

typedef enum
{
    SAVINGS,
    CURRENT
} AccountType;

struct Account
{
    int accountNumber;
    char accountHolder[100];
    char password[20];
    float balance;
    AccountType type;
};

struct Account accounts[MAX_ACCOUNTS];
int totalAccounts = 0;

// ---------- Utility Functions ----------
void clearInputBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

const char *getAccountTypeName(AccountType type)
{
    return type == SAVINGS ? "Savings" : "Current";
}

void getHiddenPassword(char *password)
{
    int i = 0;
    char ch;
    while ((ch = getch()) != '\r' && i < 19)
    {
        if (ch == '\b' && i > 0)
        {
            printf("\b \b");
            i--;
        }
        else if (ch != '\b')
        {
            password[i++] = ch;
            printf("*");
        }
    }
    password[i] = '\0';
    printf("\n");
}

// ---------- File I/O ----------
void loadAccounts()
{
    FILE *file = fopen(DATA_FILE, "rb");
    if (file)
    {
        fread(&totalAccounts, sizeof(int), 1, file);
        fread(accounts, sizeof(struct Account), totalAccounts, file);
        fclose(file);
    }
}

void saveAccounts()
{
    FILE *file = fopen(DATA_FILE, "wb");
    if (file)
    {
        fwrite(&totalAccounts, sizeof(int), 1, file);
        fwrite(accounts, sizeof(struct Account), totalAccounts, file);
        fclose(file);
    }
}

void logTransaction(int accNum, const char *action, float amount)
{
    if (amount <= 0)
        return;

    FILE *log = fopen(LOG_FILE, "a");
    if (log)
    {
        time_t now = time(NULL);
        fprintf(log, "%d,%s,%.2f,%s", accNum, action, amount, ctime(&now));
        fclose(log);
    }
}

// ---------- Core Logic ----------
int findAccount(int accNum)
{
    for (int i = 0; i < totalAccounts; i++)
    {
        if (accounts[i].accountNumber == accNum)
            return i;
    }
    return -1;
}

int authenticate(int index)
{
    char inputPass[20];
    printf("Enter password: ");
    getHiddenPassword(inputPass);
    return strcmp(inputPass, accounts[index].password) == 0;
}

void createAccount()
{
    if (totalAccounts >= MAX_ACCOUNTS)
    {
        printf("Max account limit reached.\n");
        return;
    }

    struct Account acc;
    acc.accountNumber = 1000 + totalAccounts;
    printf("Account number assigned: %d\n", acc.accountNumber);
    clearInputBuffer();

    printf("Enter account holder name: ");
    fgets(acc.accountHolder, sizeof(acc.accountHolder), stdin);
    acc.accountHolder[strcspn(acc.accountHolder, "\n")] = '\0';

    printf("Set account password: ");
    getHiddenPassword(acc.password);

    printf("Select account type (0 for Savings, 1 for Current): ");
    int type;
    scanf("%d", &type);
    acc.type = type == 1 ? CURRENT : SAVINGS;

    acc.balance = 0.0;
    accounts[totalAccounts++] = acc;
    printf("Account created successfully!\n");
}

void deposit()
{
    int accNum;
    float amount;
    printf("Enter account number: ");
    scanf("%d", &accNum);

    int index = findAccount(accNum);
    if (index == -1 || !authenticate(index))
    {
        printf("Authentication failed.\n");
        return;
    }

    printf("Enter deposit amount: ");
    scanf("%f", &amount);
    if (amount > 0)
    {
        accounts[index].balance += amount;
        logTransaction(accNum, "Deposit", amount);
        printf("Deposited ₹%.2f\n", amount);
    }
    else
    {
        printf("Invalid amount.\n");
    }
}

void withdraw()
{
    int accNum;
    float amount;
    printf("Enter account number: ");
    scanf("%d", &accNum);

    int index = findAccount(accNum);
    if (index == -1 || !authenticate(index))
    {
        printf("Authentication failed.\n");
        return;
    }

    printf("Enter withdrawal amount: ");
    scanf("%f", &amount);
    if (amount > 0 && amount <= accounts[index].balance)
    {
        accounts[index].balance -= amount;
        logTransaction(accNum, "Withdraw", amount);
        printf("Withdrawn ₹%.2f\n", amount);
    }
    else
    {
        printf("Invalid amount or insufficient balance.\n");
    }
}

void displayAccountInfo()
{
    int accNum;
    printf("Enter account number: ");
    scanf("%d", &accNum);

    int index = findAccount(accNum);
    if (index == -1 || !authenticate(index))
    {
        printf("Authentication failed.\n");
        return;
    }

    printf("\n--- Account Info ---\n");
    printf("Account Number : %d\n", accounts[index].accountNumber);
    printf("Account Holder : %s\n", accounts[index].accountHolder);
    printf("Account Type   : %s\n", getAccountTypeName(accounts[index].type));
    printf("Balance        : ₹%.2f\n", accounts[index].balance);
}

void calculateInterest()
{
    int accNum;
    printf("Enter account number: ");
    scanf("%d", &accNum);

    int index = findAccount(accNum);
    if (index == -1 || !authenticate(index))
    {
        printf("Authentication failed.\n");
        return;
    }

    float rate = accounts[index].type == SAVINGS ? 0.04 : 0.02;
    float interest = accounts[index].balance * rate;
    printf("Estimated annual interest: ₹%.2f\n", interest);
}

void viewTransactionHistory()
{
    FILE *log = fopen(LOG_FILE, "r");
    if (!log)
    {
        printf("No transaction history found.\n");
        return;
    }

    char line[200];
    printf("\n--- Transaction History ---\n");
    while (fgets(line, sizeof(line), log))
    {
        printf("%s", line);
    }
    fclose(log);
}

// ---------- Admin Features ----------
int adminLogin()
{
    char pass[20];
    printf("Enter admin password: ");
    getHiddenPassword(pass);
    return strcmp(pass, ADMIN_PASSWORD) == 0;
}

void viewAllAccounts()
{
    if (!adminLogin())
    {
        printf("Access denied.\n");
        return;
    }

    printf("\n--- All Accounts ---\n");
    for (int i = 0; i < totalAccounts; i++)
    {
        printf("Acc No: %d | Name: %s | Balance: ₹%.2f | Type: %s\n",
               accounts[i].accountNumber,
               accounts[i].accountHolder,
               accounts[i].balance,
               getAccountTypeName(accounts[i].type));
    }
}

void exportToCSV()
{
    if (!adminLogin())
    {
        printf("Access denied.\n");
        return;
    }

    FILE *file = fopen("accounts.csv", "w");
    if (!file)
    {
        printf("Error creating CSV file.\n");
        return;
    }

    fprintf(file, "AccountNumber,AccountHolder,Balance,Type\n");
    for (int i = 0; i < totalAccounts; i++)
    {
        fprintf(file, "%d,%s,%.2f,%s\n",
                accounts[i].accountNumber,
                accounts[i].accountHolder,
                accounts[i].balance,
                getAccountTypeName(accounts[i].type));
    }

    fclose(file);
    printf("Exported to accounts.csv successfully.\n");
}

void resetPassword()
{
    if (!adminLogin())
    {
        printf("Access denied.\n");
        return;
    }

    int accNum;
    printf("Enter account number to reset password: ");
    scanf("%d", &accNum);
    int index = findAccount(accNum);
    if (index == -1)
    {
        printf("Account not found.\n");
        return;
    }

    printf("Enter new password for %s: ", accounts[index].accountHolder);
    getHiddenPassword(accounts[index].password);
    printf("Password reset successful.\n");
}

// ---------- Main Menu ----------
int main()
{
    int choice;
    loadAccounts();

    while (1)
    {
        printf("\n==== Bank Account System ====\n");
        printf("1. Create Account\n");
        printf("2. Deposit\n");
        printf("3. Withdraw\n");
        printf("4. Display Account Info\n");
        printf("5. Calculate Interest\n");
        printf("6. View Transaction History\n");
        printf("7. View All Accounts (Admin)\n");
        printf("8. Export to CSV (Admin)\n");
        printf("9. Reset Password (Admin)\n");
        printf("10. Save & Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input.\n");
            clearInputBuffer();
            continue;
        }

        switch (choice)
        {
        case 1:
            createAccount();
            break;
        case 2:
            deposit();
            break;
        case 3:
            withdraw();
            break;
        case 4:
            displayAccountInfo();
            break;
        case 5:
            calculateInterest();
            break;
        case 6:
            viewTransactionHistory();
            break;
        case 7:
            viewAllAccounts();
            break;
        case 8:
            exportToCSV();
            break;
        case 9:
            resetPassword();
            break;
        case 10:
            saveAccounts();
            printf("Data saved. Exiting...\n");
            return 0;
        default:
            printf("Invalid choice. Try again.\n");
        }
    }
}
