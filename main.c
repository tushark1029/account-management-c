#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// --- Encryption-related constants ---
const int caesarkey = 19;
const char substitution_key[] = "PGWHSXCTIRZQLJOAMUYVNDFEBK";

// --- Global account data ---
int amount = 0;
int transferIds[1000];
int transferAmounts[1000];
int transferCount = 0;

int currentLoggedInAccount = -1;




// --- Encryption Logic ---
char substitute_char(char c, const char key[]) {
    c = toupper(c);
    return key[c - 'A'];
}

char* encrypt(char *passkey) {
    int length = strlen(passkey);
    char *encrypted_password = (char *)malloc(length + 1);
    if (!encrypted_password) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    for (int i = 0; i < length; i++) {
        if (islower(passkey[i])) {
            int x = (passkey[i] - 'a' + caesarkey) % 26;
            encrypted_password[i] = tolower(substitute_char('a' + x, substitution_key));
        } else if (isupper(passkey[i])) {
            int x = (passkey[i] - 'A' + caesarkey) % 26;
            encrypted_password[i] = substitute_char('A' + x, substitution_key);
        } else {
            encrypted_password[i] = passkey[i];
        }
    }
    encrypted_password[length] = '\0';
    return encrypted_password;
}



// --- Signup Logic ---
void signup() {
    char name[100], phone[20], email[100], dob[20], gender[10], password[100];
    int account_number;
    FILE *file;

    getchar(); // Clear newline

    printf("\nEnter your full name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    printf("Enter phone number: ");
    fgets(phone, sizeof(phone), stdin);
    phone[strcspn(phone, "\n")] = 0;

    printf("Enter email address: ");
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = 0;

    printf("Enter date of birth (dd/mm/yyyy): ");
    fgets(dob, sizeof(dob), stdin);
    dob[strcspn(dob, "\n")] = 0;

    printf("Enter gender: ");
    fgets(gender, sizeof(gender), stdin);
    gender[strcspn(gender, "\n")] = 0;

    printf("Choose a password: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;

    char *encrypted = encrypt(password);

    // --- Generate a unique 3-digit account number ---
    srand(time(NULL));
    int is_unique = 0;
    do {
        account_number = rand() % 900 + 100; // generates a number from 100 to 999
        is_unique = 1;
        file = fopen("users.txt", "r");
        if (file) {
            char line[300], file_acc[10];
            while (fgets(line, sizeof(line), file)) {
                if (sscanf(line, "%[^,]", file_acc) == 1) {
                    if (atoi(file_acc) == account_number) {
                        is_unique = 0;
                        break;
                    }
                }
            }
            fclose(file);
        }
    } while (!is_unique);

    // --- Save user to users.txt ---
    file = fopen("users.txt", "a");
    if (!file) {
        printf("Error: Cannot open users.txt for writing.\n");
        free(encrypted);
        return;
    }
    fprintf(file, "%d,%s,%s,%s,%s,%s,%s\n", account_number, encrypted, name, phone, email, dob, gender);
    fclose(file);

    // --- Add entry to accounts.txt with 0 balance ---
    file = fopen("accounts.txt", "a");
    if (!file) {
        printf("Error: Cannot open accounts.txt for writing.\n");
        free(encrypted);
        return;
    }
    fprintf(file, "%d,0\n", account_number);
    fclose(file);

    free(encrypted);
    printf("\nSignup successful! Your account number is: %d\n", account_number);
}




// --- Banking Dashboard Logic ---
void view_balance() {
    if (currentLoggedInAccount == -1) {
        printf("You must be logged in to view balance.\n");
        return;
    }

    FILE *file = fopen("accounts.txt", "r");
    if (!file) {
        printf("Error: Cannot open accounts.txt\n");
        return;
    }

    int fileAcc, balance;
    char line[100];
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d,%d", &fileAcc, &balance) == 2) {
            if (fileAcc == currentLoggedInAccount) {
                printf("Your Account Balance: %d\n", balance);
                found = 1;
                break;
            }
        }
    }

    fclose(file);

    if (!found) {
        printf("Account not found.\n");
    }
}




void deposit() {
    float depositAmount;
    char line[300];

  
    printf("Enter amount to deposit: ");
    if (scanf("%f", &depositAmount) != 1 || depositAmount <= 0) {
        printf("Invalid amount!\n");
        while (getchar() != '\n');
        return;
    }

    FILE *in = fopen("accounts.txt", "r");
    FILE *out = fopen("temp_accounts.txt", "w");

    if (!in || !out) {
        printf("Error accessing accounts file.\n");
        if (in) fclose(in);
        if (out) fclose(out);
        return;
    }

    int fileAcc, balance;
    int found = 0;

    while (fgets(line, sizeof(line), in)) {
        if (sscanf(line, "%d,%d", &fileAcc, &balance) == 2) {
            if (fileAcc == currentLoggedInAccount) {
                balance += depositAmount;
                found = 1;
                printf("Deposited %.2f. New Balance: %d\n", depositAmount, balance);
            }
            fprintf(out, "%d,%d\n", fileAcc, balance);
        }
    }

    fclose(in);
    fclose(out);
    remove("accounts.txt");
    rename("temp_accounts.txt", "accounts.txt");

    if (!found)
        printf("Account number not found.\n");

    while (getchar() != '\n');
}



void withdraw() {
    int withdrawAmount;
    char line[300];

    printf("Enter amount to withdraw: ");
    if (scanf("%d", &withdrawAmount) != 1 || withdrawAmount <= 0) {
        printf("Invalid amount!\n");
        while (getchar() != '\n');
        return;
    }

    FILE *in = fopen("accounts.txt", "r");
    FILE *out = fopen("temp_accounts.txt", "w");

    if (!in || !out) {
        printf("Error accessing accounts file.\n");
        if (in) fclose(in);
        if (out) fclose(out);
        return;
    }

    int fileAcc, balance;
    int found = 0;

    while (fgets(line, sizeof(line), in)) {
        if (sscanf(line, "%d,%d", &fileAcc, &balance) == 2) {
            if (fileAcc == currentLoggedInAccount) {
                if (balance < withdrawAmount) {
                    printf("Insufficient balance.\n");
                    fprintf(out, "%d,%d\n", fileAcc, balance); // Write unchanged
                    found = 1;
                    continue;
                }
                balance -= withdrawAmount;
                printf("Withdrawn %d. New Balance: %d\n", withdrawAmount, balance);
                found = 1;
            }
            fprintf(out, "%d,%d\n", fileAcc, balance);
        }
    }

    fclose(in);
    fclose(out);
    remove("accounts.txt");
    rename("temp_accounts.txt", "accounts.txt");

    if (!found)
        printf("Account number not found.\n");

    while (getchar() != '\n');
}



// --- Transfer Logic ---
void transfer() {
    int recipientAcc, transferAmount;
    char password[100];
    char line[300];
    char file_pass[100];
    int acc_no;
    int validRecipient = 0;

    printf("\n=== Transfer Money ===\n");

    printf("Enter recipient's account number: ");
    if (scanf("%d", &recipientAcc) != 1) {
        printf("Invalid recipient account number.\n");
        while (getchar() != '\n');
        return;
    }

    // --- Check if recipient exists ---
    FILE *accFile = fopen("accounts.txt", "r");
    if (!accFile) {
        printf("Error: Cannot open accounts.txt\n");
        return;
    }

    while (fgets(line, sizeof(line), accFile)) {
        int fileAcc;
        if (sscanf(line, "%d", &fileAcc) == 1 && fileAcc == recipientAcc) {
            validRecipient = 1;
            break;
        }
    }
    fclose(accFile);

    if (!validRecipient) {
        printf("Recipient account not found.\n");
        return;
    }

    printf("Enter amount to transfer: ");
    if (scanf("%d", &transferAmount) != 1 || transferAmount <= 0) {
        printf("Invalid amount.\n");
        while (getchar() != '\n');
        return;
    }

    while (getchar() != '\n'); // clear leftover input
    printf("Enter your password to confirm: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = 0;
    char *encInput = encrypt(password);

    // --- Check password from users.txt ---
    FILE *userFile = fopen("users.txt", "r");
    int passwordCorrect = 0;

    if (userFile) {
        while (fgets(line, sizeof(line), userFile)) {
            if (sscanf(line, "%d,%[^,]", &acc_no, file_pass) == 2) {
                if (acc_no == currentLoggedInAccount && strcmp(file_pass, encInput) == 0) {
                    passwordCorrect = 1;
                    break;
                }
            }
        }
        fclose(userFile);
    }

    time_t now = time(NULL);
    char *datetime = ctime(&now);
    datetime[strcspn(datetime, "\n")] = 0;

    FILE *txnFile = fopen("transactions.txt", "a");

    if (!passwordCorrect) {
        printf("Incorrect password. Transfer failed.\n");
        if (txnFile)
            fprintf(txnFile, "%d,%d,%d,%s,Failed\n", recipientAcc, currentLoggedInAccount, transferAmount, datetime);
        free(encInput);
        if (txnFile) fclose(txnFile);
        return;
    }

    // --- Update accounts.txt balances ---
    FILE *accRead = fopen("accounts.txt", "r");
    FILE *accWrite = fopen("temp_accounts.txt", "w");

    if (!accRead || !accWrite) {
        printf("Error processing accounts file.\n");
        free(encInput);
        if (txnFile) fclose(txnFile);
        return;
    }

    int fileAcc, balance;
    int senderFound = 0, recipientFound = 0;
    int insufficientFunds = 0;

    while (fgets(line, sizeof(line), accRead)) {
        if (sscanf(line, "%d,%d", &fileAcc, &balance) == 2) {
            if (fileAcc == currentLoggedInAccount) {
                senderFound = 1;
                if (balance < transferAmount) {
                    insufficientFunds = 1;
                    fprintf(accWrite, "%d,%d\n", fileAcc, balance); // unchanged
                } else {
                    balance -= transferAmount;
                    fprintf(accWrite, "%d,%d\n", fileAcc, balance);
                }
            } else if (fileAcc == recipientAcc) {
                recipientFound = 1;
                if (!insufficientFunds) balance += transferAmount;
                fprintf(accWrite, "%d,%d\n", fileAcc, balance);
            } else {
                fprintf(accWrite, "%d,%d\n", fileAcc, balance);
            }
        }
    }

    fclose(accRead);
    fclose(accWrite);
    remove("accounts.txt");
    rename("temp_accounts.txt", "accounts.txt");

    if (insufficientFunds) {
        printf("Insufficient balance. Transfer failed.\n");
        if (txnFile)
            fprintf(txnFile, "%d,%d,%d,%s,Failed\n", recipientAcc, currentLoggedInAccount, transferAmount, datetime);
    } else if (senderFound && recipientFound) {
        printf("Transfer successful!\n");
        if (txnFile)
            fprintf(txnFile, "%d,%d,%d,%s,Success\n", recipientAcc, currentLoggedInAccount, transferAmount, datetime);
    }

    free(encInput);
    if (txnFile) fclose(txnFile);
}



void transfer_history() {
    if (currentLoggedInAccount == -1) {
        printf("You must be logged in to view transfer history.\n");
        return;
    }

    printf("\n=== Transfer History ===\n");

    FILE *file = fopen("transactions.txt", "r");
    if (!file) {
        printf("No transaction history available.\n");
        return;
    }

    char line[300];
    int recipient, sender, amount;
    char datetime[100], status[20];
    int found = 0;

    while (fgets(line, sizeof(line), file)) {
        // Match: Recipient, Sender, Amount, DateTime, Status
        if (sscanf(line, "%d,%d,%d,%[^,],%s", &recipient, &sender, &amount, datetime, status) == 5) {
            if (sender == currentLoggedInAccount || recipient == currentLoggedInAccount) {
                found = 1;
                printf("To: %d | From: %d | Amount: %d | Date: %s | Status: %s\n",
                       recipient, sender, amount, datetime, status);
            }
        }
    }

    fclose(file);

    if (!found) {
        printf("No transfers found for your account.\n");
    }
}






void dashboard(const char* username) {
    int choice;
    while (1) {
        printf("\n====== Welcome, %s ======\n", username);
        printf("1) View Account Balance\n");
        printf("2) Deposit\n");
        printf("3) Withdraw\n");
        printf("4) Transfer\n");
        printf("5) Transfer History\n");
        printf("6) Logout\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input!\nPlease enter a number.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1:
                view_balance();
                break;
            case 2:
                deposit();
                break;
            case 3:
                withdraw();
                break;
            case 4:
                transfer();
                break;
            case 5:
                transfer_history();
                break;
            case 6:
                printf("Logging out...\n");
                return;
            default:
                printf("Invalid choice!\n");
        }
    }
}

// --- Login / Authentication Logic ---
int authenticate() {
    FILE *file = fopen("users.txt", "r");
    if (!file) {
        printf("Error: Cannot open users.txt\n");
        return 0;
    }

    int input_acc_no;
    char input_password[100];
    int attempts = 0;
    time_t lock_until = 0;

    while (1) {
        time_t now = time(NULL);
        if (now < lock_until) {
            printf("Account locked. Try again in %ld seconds.\n", lock_until - now);
            continue;
        }

        printf("\nEnter Account Number: ");
        if (scanf("%d", &input_acc_no) != 1) {
            printf("Invalid input! Account number must be numeric.\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n'); // clear input buffer

        printf("Enter Password: ");
        fgets(input_password, sizeof(input_password), stdin);
        input_password[strcspn(input_password, "\n")] = 0;

        char *encrypted_input = encrypt(input_password);

        rewind(file);
        char line[300], file_pass[100], name[100];
        int file_acc_no;
        int found = 0;

        while (fgets(line, sizeof(line), file)) {
            // Extract account number and encrypted password from each line
            if (sscanf(line, "%d,%99[^,],%99[^,\n]", &file_acc_no, file_pass, name) >= 2) {
                if (file_acc_no == input_acc_no &&
                    strcmp(encrypted_input, file_pass) == 0) {
                    found = 1;
                    break;
                }
            }
        }

        free(encrypted_input);

        if (found) {
            currentLoggedInAccount = input_acc_no; // Store the account number globally
            printf("Login successful! Welcome, %s\n", name);
            fclose(file);
            dashboard(name);  // Use name in dashboard welcome
            return 1;
        } else {
            printf("Invalid account number or password.\n");
            attempts++;
            if (attempts >= 3) {
                printf("Too many failed attempts. Locking for 30 seconds.\n");
                lock_until = time(NULL) + 30;
                attempts = 0;
            }
        }
    }
}


// --- Main Menu ---
int main() {
    int choice;
    printf("=== Welcome to the Banking System ===\n");
    do {
        printf("\n1) Login\n2) Sign Up\n3) Exit\nChoose an option: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }

        switch (choice) {
            case 1:
                authenticate();
                break;
            case 2:
                signup();
                break;
            case 3:
                printf("Exiting the system. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Try again.\n");
        }
    } while (choice != 3);

    return 0;
}
