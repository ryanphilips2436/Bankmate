#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <conio.h> 
#include "sqlite/sqlite3.c"
#include "sqlite/sqlite3.h"

sqlite3 *db;
sqlite3_stmt *stmt;
int rc;

int choice();
int admin_options();
void admin();
void create_acc();
void customer_details();
void delete_acc();
void logout();
void customers();
void get_pin(char *pin, int max_len);
void to_uppercase(char *str);

//=======================================START PAGE============================================
int choice() {
    int option;
    printf("      _______________________________________________________________________ \n");
    printf("     |                                                                       |\n");
    printf("     |                         1-ADMIN LOGIN                                 |\n");
    printf("     |                         2-CUSTOMER LOGIN                              |\n");
    printf("     |                         3-EXIT                                        |\n");
    printf("     |_______________________________________________________________________|\n");
    printf("                               ENTER CHOICE:");
    scanf("%d", &option);
    printf("     |_______________________________________________________________________|\n\n");

    if (option<4 && option>0)
        return option;
    else
        return choice();
}

//===================================ADMIN===================================================
int admin_option(){
    int option;
    printf("\n     |  1-CREATE BANK ACCOUNT                                                |\n");
    printf("     |  2-CUSTOMER ACCOUNT DETAILS                                           |\n");
    printf("     |  3-DELETE BANK ACCOUNT                                                |\n");
    printf("     |  4-LOG OUT                                                            |\n");
    printf("     |_______________________________________________________________________|\n");
    printf("     |      ENTER CHOICE:");
    scanf("%d", &option);
    printf("     |_______________________________________________________________________|\n\n");

    if (option<5 && option>0){
        return option;}
    else{
        printf("    INVALID INPUT\n\n");
        return admin_option();
    }
}

void admin() {
    char user[50], pswrd[100];
    int option;
    printf("      _______________________________________________________________________\n");
    printf("     |                                                                       |\n");
    printf("     |                         ADMIN LOGIN PANEL                             |\n");
    printf("     |_______________________________________________________________________|\n");
    printf("     |                                                                       |\n");
    printf("     |  USERNAME : "); scanf("%s", user);
    printf("     |  PASSWORD : "); scanf("%s", pswrd);
    printf("     |_______________________________________________________________________|\n");


    // Prepare SQL query to fetch admin by username and password
    const char *sql = "SELECT admin_id, username, password FROM admins WHERE username=? AND password=?;";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Bind user inputs to the SQL statement
    sqlite3_bind_text(stmt, 1, user, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, pswrd, -1, SQLITE_TRANSIENT);


    // Execute the query
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int admin_id = sqlite3_column_int(stmt, 0);
        const unsigned char *uname = sqlite3_column_text(stmt, 1);
        const unsigned char *pwd = sqlite3_column_text(stmt, 2);

        printf("     |                                                                       |\n");
        printf("     |  Login Successful!\n");
        printf("     |  Admin ID   : %d                                                    |\n", admin_id);
    } else {
        printf("     |  Invalid username or password.                                        |\n");
        admin();
    }
    sqlite3_finalize(stmt);

    printf("     |  Welcome, %s! \n", user);
    option=admin_option();
    switch (option) {
        case 1: create_acc(); break;
        case 2: customer_details(); break;
        case 3: delete_acc(); break;
        case 4: logout(); break;
        default: break;
    }
}


//===========view customer details====================
void view_details(const char *search_name){
    char upper_name[50];
    strcpy(upper_name, search_name);
    to_uppercase(upper_name);
    const char *sql = "SELECT account_no, name, dob, aadhaar, mobile, email, address, account_type, balance "
                      "FROM customers WHERE name = ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Bind the name parameter to the SQL statement
    sqlite3_bind_text(stmt, 1, upper_name, -1, SQLITE_STATIC);
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int accno = sqlite3_column_int(stmt, 0);
        const unsigned char *name = sqlite3_column_text(stmt, 1);
        const unsigned char *dob = sqlite3_column_text(stmt, 2);
        const unsigned char *aadhaar = sqlite3_column_text(stmt, 3);
        const unsigned char *mobile = sqlite3_column_text(stmt, 4);
        const unsigned char *email = sqlite3_column_text(stmt, 5);
        const unsigned char *address = sqlite3_column_text(stmt, 6);
        const unsigned char *account_type = sqlite3_column_text(stmt, 7);
        double balance = sqlite3_column_double(stmt, 8);

        int len = strlen(aadhaar);
        int i, j = 0;
        char output[20];
        for (i = 0; i < len; i++) {
            if (i < len - 4) {
                output[j++] = 'X';
            } else {
                output[j++] = aadhaar[i];
            }
            // Add space after every 4 characters (except at the very end)
            if ((i + 1) % 4 == 0 && i != len - 1) {
                output[j++] = ' ';
            }
        }
        output[j] = '\0';

        printf("     |  ACCOUNT NO.     : %lld\n", accno);
        printf("     |  NAME            : %s\n", name);
        printf("     |  DATE OF BIRTH   : %s\n", dob);
        printf("     |  AADHAR NUMBER   : %s\n", output);
        printf("     |  MOBILE NUMBER   : +91 %s\n", mobile);
        printf("     |  EMAIL           : %s\n", email);
        printf("     |  ADDRESS         : %s\n", address);
        printf("     |  BALANCE         : %.2f\n", balance);
        printf("     |_______________________________________________________________________|\n\n");
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "SELECT error: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

//================================CUSTOMER==================================================
void customers() {
    char name[50], pin[10], db_pin[20];
    long long acc_no;
    int login_success = 0, attempts = 0;
    printf("      _______________________________________________________________________ \n");
    printf("     |                                                                       |\n");
    printf("     |                         CUSTOMER LOGIN PANEL                          |\n");
    printf("     |_______________________________________________________________________|\n");

    while (!login_success && attempts < 3) {
        printf("     |  NAME         : "); scanf(" %[^\n]", name);
        printf("     |  ACCOUNT NO.  : "); scanf(" %lld", &acc_no);
        printf("     |  PIN          : "); get_pin(pin, sizeof(pin));
        char upper_name[50];
        strcpy(upper_name, name); 
        to_uppercase(upper_name);

        const char *sql = "SELECT pin FROM customers WHERE name=? AND account_no=?;";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc != SQLITE_OK) { printf("DB error: %s\n", sqlite3_errmsg(db)); return; }
        sqlite3_bind_text(stmt, 1, upper_name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, acc_no);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            strcpy(db_pin, (const char*)sqlite3_column_text(stmt, 0));
            int stored_pin = atoi(db_pin) >> 3;
            if (atoi(pin) == stored_pin) {
                login_success = 1;
            } else {
                printf("     |  Incorrect PIN. Try again.                                            |\n");
            }
        } else {
            printf("     |  Invalid name or account number.                                      |\n");
        }
        sqlite3_finalize(stmt);
        attempts++;
    }

    if (!login_success){ 
        printf("     |  Login failed.\n"); 
        return;
    }

    int cust_menu = 0;
    printf("     |  Login successful! Welcome, %s!\n\n", name);
    view_details(name);

    while (1) {
        printf("      _______________________________________________________________________ \n");
        printf("     |                                                                       |\n");
        printf("     |  1-View Last 10 Transactions                                          |\n");
        printf("     |  2-View Balance                                                       |\n");
        printf("     |  3-Make Transaction                                                   |\n");
        printf("     |  4-Logout                                                             |\n");
        printf("     |  Enter choice: ");     scanf("%d", &cust_menu);
        printf("     |_______________________________________________________________________|\n");

        if (cust_menu < 1 || cust_menu > 4) {
            printf("     |  Invalid choice. Try again.                                           |\n");
            continue;
        }

        // View last 10 transactions
        if (cust_menu == 1) {
            // Join with customers table to get names for both sender and receiver
            const char *sql = "SELECT t.txn_time, c1.name as from_name, c2.name as to_name, t.amount, t.remarks "
                              "FROM transactions t "
                              "LEFT JOIN customers c1 ON t.from_account_no = c1.account_no "
                              "LEFT JOIN customers c2 ON t.to_account_no = c2.account_no "
                              "WHERE t.from_account_no=? OR t.to_account_no=? "
                              "ORDER BY t.txn_time DESC LIMIT 10;";
            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

            if (rc != SQLITE_OK){
                printf("DB error: %s\n", sqlite3_errmsg(db)); 
                continue; 
            }
            sqlite3_bind_int64(stmt, 1, acc_no);
            sqlite3_bind_int64(stmt, 2, acc_no);
            printf("\n     ______________________________________________________________________________________________________\n");
            printf("    |  Date/Time           |  From                |  To                  |  Amount   |  Remarks            |\n");
            printf("    |----------------------|----------------------|----------------------|-----------|---------------------|\n");
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                const char *dt = (const char*)sqlite3_column_text(stmt, 0);
                const char *from_name = (const char*)sqlite3_column_text(stmt, 1);
                const char *to_name = (const char*)sqlite3_column_text(stmt, 2);
                double amt = sqlite3_column_double(stmt, 3);
                const char *remarks = (const char*)sqlite3_column_text(stmt, 4);
                printf("    | %-20s | %-20s | %-20s | %-8.2f  | %s\n", dt, from_name ? from_name : "-", to_name ? to_name : "-", amt, remarks ? remarks : "");
            }
            sqlite3_finalize(stmt);
            printf("    |_____________________________________________________________________________________________________|\n\n");
        

        // =======================View balance=====================   
        }else if (cust_menu == 2){
            const char *sql = "SELECT balance FROM customers WHERE account_no=?;";
            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

            if (rc != SQLITE_OK){
                printf("DB error: %s\n", sqlite3_errmsg(db)); 
                continue;
            }
            sqlite3_bind_int64(stmt, 1, acc_no);
            if (sqlite3_step(stmt) == SQLITE_ROW){
                double bal = sqlite3_column_double(stmt, 0);
                printf("\n     |  Current Balance: %.2f\n", bal);
            }
            sqlite3_finalize(stmt);


        // =======================Make transaction===========================
        } else if (cust_menu == 3) {
            char recv_name[50], recv_name_up[50], recv_last4[10];
            long long recv_acc = 0;
            double amount = 0.0, sender_bal = 0.0;
            double recv_bal = 0.0;
            char pin_check[10];

            printf("     |  Enter receiver's name: "); scanf(" %[^\n]", recv_name);
            strcpy(recv_name_up, recv_name); to_uppercase(recv_name_up);

            printf("     |  Enter last 4 digits of receiver's account: "); scanf("%s", recv_last4);
            if (strlen(recv_last4) != 4 || !isdigit(recv_last4[0]) || !isdigit(recv_last4[1]) || !isdigit(recv_last4[2]) || !isdigit(recv_last4[3])) {
                printf("     |  Invalid input. Must be exactly 4 digits.\n");
                continue;
            }

            // Find receiver's account
            const char *sql = "SELECT account_no, balance FROM customers WHERE name=?;";
            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

            if (rc != SQLITE_OK){
                printf("DB error: %s\n", sqlite3_errmsg(db)); 
                continue; 
            }
            sqlite3_bind_text(stmt, 1, recv_name_up, -1, SQLITE_TRANSIENT);
            int found = 0;
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                long long acc = sqlite3_column_int64(stmt, 0);
                char acc_str[20]; sprintf(acc_str, "%lld", acc);
                int len = strlen(acc_str);
                if (len >= 4 && strcmp(acc_str + len - 4, recv_last4) == 0) {
                    recv_acc = acc;
                    recv_bal = sqlite3_column_double(stmt, 1);
                    found = 1;
                    break;
                }
            }

            sqlite3_finalize(stmt);
            if (!found){
                printf("     |  Receiver not found.\n");
                continue;
            }
            if (recv_acc == acc_no) {
                printf("     |  Cannot transfer to the same account.\n");
                continue;
            }

            // Get sender balance
            sql = "SELECT balance FROM customers WHERE account_no=?;";
            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

            if (rc != SQLITE_OK){
                printf("DB error: %s\n", sqlite3_errmsg(db));
                continue;
            }
            sqlite3_bind_int64(stmt, 1, acc_no);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                sender_bal = sqlite3_column_double(stmt, 0);
            }
            sqlite3_finalize(stmt);

            printf("     |  Enter amount to send: "); scanf("%lf", &amount);
            if (amount <= 0 || amount > sender_bal){ 
                printf("     |  Insufficient balance or invalid amount.\n"); 
                continue; 
            }
            printf("     |  Enter your PIN to confirm: "); 
            get_pin(pin_check, sizeof(pin_check));
            int stored_pin = atoi(db_pin) >> 3;
            if (atoi(pin_check) != stored_pin){
                printf("     |  Incorrect PIN. Transaction cancelled.\n");
                continue;
            }

        
            // Perform transaction
            printf("\n     |  Transferring %.2f to %s (Acc: %lld)...\n", amount, recv_name, recv_acc);

            // Deduct from sender
            sql = "UPDATE customers SET balance=balance-? WHERE account_no=?;";
            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            if (rc != SQLITE_OK){
                printf("DB error: %s\n", sqlite3_errmsg(db));
                continue;
            }
            sqlite3_bind_double(stmt, 1, amount);
            sqlite3_bind_int64(stmt, 2, acc_no);
            if (sqlite3_step(stmt) != SQLITE_DONE){
                printf("     |  Error updating sender balance.\n");
                sqlite3_finalize(stmt);
                continue;
            }
            sqlite3_finalize(stmt);

            // Add to receiver
            sql = "UPDATE customers SET balance=balance+? WHERE account_no=?;";
            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
                printf("DB error: %s\n", sqlite3_errmsg(db));
                continue;
            }
            sqlite3_bind_double(stmt, 1, amount);
            sqlite3_bind_int64(stmt, 2, recv_acc);
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                printf("     |  Error updating receiver balance.\n");
                sqlite3_finalize(stmt);
                continue;
            }
            sqlite3_finalize(stmt);
            
            // Insert transaction record
            char remarks[100] = "TRANSFER";
            sql = "INSERT INTO transactions (from_account_no, to_account_no, amount, remarks) VALUES (?, ?, ?, ?);";
            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
                printf("DB error: %s\n", sqlite3_errmsg(db));
                continue;
            }
            sqlite3_bind_int64(stmt, 1, acc_no);
            sqlite3_bind_int64(stmt, 2, recv_acc);
            sqlite3_bind_double(stmt, 3, amount);
            sqlite3_bind_text(stmt, 4, remarks, -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                printf("     |  Transaction successful!\n");
            } else {
                printf("     |  Error recording transaction.\n");
            }
            sqlite3_finalize(stmt);

        } else if (cust_menu == 4) {
            logout();
            return;
        } else {
            printf("     |  Invalid choice.\n");
        }
    }
    return;
}

// ===================== Verhoeff Algorithm =====================
//used to verify aadhaar number

// multiplication table
int d[10][10] = {
    {0,1,2,3,4,5,6,7,8,9},
    {1,2,3,4,0,6,7,8,9,5},
    {2,3,4,0,1,7,8,9,5,6},
    {3,4,0,1,2,8,9,5,6,7},
    {4,0,1,2,3,9,5,6,7,8},
    {5,9,8,7,6,0,4,3,2,1},
    {6,5,9,8,7,1,0,4,3,2},
    {7,6,5,9,8,2,1,0,4,3},
    {8,7,6,5,9,3,2,1,0,4},
    {9,8,7,6,5,4,3,2,1,0}
};

// permutation table
int p[8][10] = {
    {0,1,2,3,4,5,6,7,8,9},
    {1,5,7,6,2,8,3,0,9,4},
    {5,8,0,3,7,9,6,1,4,2},
    {8,9,1,6,0,4,3,5,2,7},
    {9,4,5,3,1,2,6,8,7,0},
    {4,2,8,6,5,7,3,9,0,1},
    {2,7,9,3,8,0,6,4,1,5},
    {7,0,4,6,9,1,3,2,5,8}
};

// inverse table
int inv[10] = {0,4,3,2,1,5,6,7,8,9};
int verhoeff_checksum(const char *num) {
    int c = 0;
    int len = strlen(num);
    for (int i = 0; i < len; i++) {
        int digit = num[len - 1 - i] - '0';
        c = d[c][ p[i % 8][digit] ];
    }
    return c;
}
// Validate Aadhaar (must be 12 digits and pass Verhoeff)
int validate_aadhaar(const char *aadhaar) {
    if (strlen(aadhaar) != 12) {
        return 0; // Aadhaar must be exactly 12 digits
    }
    for (int i = 0; i < 12; i++) {
        if (aadhaar[i] < '0' || aadhaar[i] > '9')
            return 0; // must contain only digits
    }
    return (verhoeff_checksum(aadhaar) == 0);
}


//convert to uppercase
void to_uppercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

//masking pin
void get_pin(char *pin, int max_len) {
    int i = 0;
    char ch;

    while (i < max_len - 1) {
        ch = getch();           // get a character without showing it

        if (ch == '\r') {       // Enter key pressed
            printf("\n");
            break;
        } else if (ch == '\b') { // Backspace pressed
            if (i > 0) {
                i--;
                printf("\b \b"); // erase last '*'
            }
        } else if (ch >= '0' && ch <= '9') {
            pin[i++] = ch;
            printf("*");        // print masking character
        }
        // ignore any other characters
    }
    pin[i] = '\0';               // null-terminate
}

// Check if Aadhaar, mobile, or email already exists for a different name
int check_existing_user(const char *name, const char *aadhaar, const char *mobile, const char *email) {
    const char *sql = "SELECT name FROM customers WHERE aadhaar = ? OR mobile = ? OR email = ?;";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return -1; // DB error
    }

    sqlite3_bind_text(stmt, 1, aadhaar, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, mobile, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, email, -1, SQLITE_TRANSIENT);

    int allowed = 1;  // assume allowed
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const unsigned char *db_name = sqlite3_column_text(stmt, 0);
        if (strcasecmp((const char*)db_name, name) != 0) {
            // name mismatch → belongs to someone else
            allowed = 0;
            break;
        }
    }

    sqlite3_finalize(stmt);
    return allowed;
}

//==================================CREATE ACCOUNT========================================
void create_acc(){
    int age,calculated_age,invalid=0;
    int birth_day, birth_month, birth_year;
    char name[50], dob[15], aadhar[20], mobile[12];
    char email[50], address[100], account_type[20];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int year = tm.tm_year + 1900;


    int row_count = 0;
    const char *sql = "SELECT COUNT(*) FROM customers";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Execute and get the result
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        row_count = sqlite3_column_int(stmt, 0);
        row_count+=1;
    }

    // Show result
    long long acc_number = (long long)year * 1000000 + row_count;
    sqlite3_finalize(stmt);

    printf("      _______________________________________________________________________\n");
    printf("     |                                                                       |\n");
    printf("     |                        ACCOUNT CREATION PANEL                         |\n");
    printf("     |_______________________________________________________________________|\n");
    printf("     |                                                                       |\n");
    printf("     |  FULL NAME         : ");      scanf(" %[^\n]", name);
    printf("     |  DOB (DD/MM/YYYY)  : ");      scanf("%d/%d/%d", &birth_day, &birth_month, &birth_year);
    printf("     |  AGE               : ");      scanf("%d", &age);
    printf("     |  AADHAR NUMBER     : ");      scanf(" %[^\n]", aadhar);
    printf("     |  MOBILE NUMBER     : +91 ");  scanf(" %[^\n]", mobile);
    printf("     |  EMAIL             : ");      scanf(" %[^\n]", email);
    printf("     |  ADDRESS           : ");      scanf(" %[^\n]", address);
    printf("     |  ACCOUNT TYPE (SAVINGS / CURRENT) : ");     scanf(" %[^\n]", account_type);
    printf("     |_______________________________________________________________________|\n\n");


    //=============verifying DOB & AGE==============
    // Get current date
    t = time(NULL);
    struct tm today = *localtime(&t);

    calculated_age = (today.tm_year + 1900) - birth_year;
    if (birth_month > (today.tm_mon+1) || 
    (birth_month == (today.tm_mon+1) && birth_day > today.tm_mday)) {
        calculated_age--;
    }

    if (calculated_age != age) {
        printf("     |  Entered age does NOT match calculated age!                           |\n");
        return;
    }
    if (age < 18) {
        printf("     |  You must be 18 or older to open an account                           |\n");
        return;
    }
    printf("     |  Age >= 18 verified                                                   |\n");
    snprintf(dob, sizeof(dob), "%02d/%02d/%04d", birth_day, birth_month, birth_year);

    //===========verifying aadhar number==========
    if (validate_aadhaar(aadhar)) {
        printf("     |  Aadhar number verified and VALID                                     |\n");
    } else {
        printf("     |  Aadhar number INVALID                                                |\n");
        invalid=1;
    }

    //=============verifying mobile no=============
    int valid = 1;
    if (strlen(mobile) != 10) {
        valid = 0;
    } else {
        // Check all characters are digits
        for (int i = 0; i < 10; i++) {
            if (!isdigit((unsigned char)mobile[i])) {
                valid = 0;
                break;
            }
        }
    }
    switch (valid){
        case 1: printf("     |  phone number VALID                                                   |\n");
                break;
        case 0: printf("     |  phone number INVALID                                                 |\n");
                invalid=2; break;
        default: break;
    }

    //===============verifying email id==================
    char gmail[] = "@gmail.com";
    char outlook[] = "@outlook.com";

    int len = strlen(email);
    int gmailLen = strlen(gmail);
    int outlookLen = strlen(outlook);

    int isValid = 0;

    // Check for gmail
    if (len > gmailLen) {
        int match = 1;
        for (int i = 0; i < gmailLen; i++) {
            if (email[len - gmailLen + i] != gmail[i]) {
                match = 0;
                break;
            }
        }
        if (match == 1) isValid = 1;
    }

    // Check for outlook
    if (len > outlookLen && isValid == 0) {
        int match = 1;
        for (int i = 0; i < outlookLen; i++) {
            if (email[len - outlookLen + i] != outlook[i]) {
                match = 0;
                break;
            }
        }
        if (match == 1) isValid = 1;
    }

    if (isValid == 1){
        printf("     |  Email id verified and VALID                                          |\n");
    }else{
        printf("     |  Email id INVALID                                                     |\n");
        invalid=3;
    }

    while (invalid != 0) {
        printf("     |  INVALID data entered. Please try again.\n");
        create_acc();
        return; 
    }

    //Fake account check
    if (!check_existing_user(name, aadhar, mobile, email)) {
        printf("\nAccount creation failed: Aadhaar/Mobile/Email already linked to another person.\n");
        create_acc();
        return;  // stop creation}
    }

    //ENTER PIN
    char pin[10],con_pin[10];
    while (1) {
        printf("     |                                                                       |\n");
        printf("     |  ENTER PIN          : ");    get_pin(pin, sizeof(pin));
        printf("     |  CONFIRM PIN        : ");    get_pin(con_pin, sizeof(con_pin));
        if (strlen(pin)!=6 || strlen(con_pin)!=6) {
            printf("     |  PIN must be exactly 6 digits long                                    |\n");
            printf("     |  Please try entering the PIN again                                    |\n");
            continue;
        }

        // Check match
        if (strcmp(pin,con_pin)!=0){
            printf("     |  PIN and Confirm PIN do not match                                     |\n");
            printf("     |  Please try entering the PIN again                                    |\n");
            continue;}
        printf("     |  PIN set successfully!                                                |\n");
        break;  
    }

    int pin_test=atoi(pin);
    int encrypt=pin_test<<3;
    char encrypted_pin[10];
    sprintf(encrypted_pin, "%d", encrypt);
    to_uppercase(name);
    to_uppercase(address);
    to_uppercase(account_type);

    const char *insert_sql = "INSERT INTO customers "
                      "(account_no, name, dob, aadhaar, mobile, email, address, account_type, pin) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

    rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_int64(stmt, 1, acc_number);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, dob, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, aadhar, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, mobile, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, email, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, address, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, account_type, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, encrypted_pin, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Insert failed: %s\n", sqlite3_errmsg(db));
    } else {
        printf("     |_______________________________________________________________________|\n\n");
        printf("     |  Account Created Successfully!                                        |\n");
    }

    sqlite3_finalize(stmt);
    view_details(name);
    int option=admin_option();
    switch (option) {
        case 1: create_acc(); break;
        case 2: customer_details(); break;
        case 3: delete_acc(); break;
        case 4: logout(); break;
        default: break;
    }
}

void customer_details(){
    char search[50];
    printf("      _______________________________________________________________________\n");
    printf("     |                                                                       |\n");
    printf("     |                        CUSTOMER DETAILS PANEL                         |\n");
    printf("     |_______________________________________________________________________|\n");
    printf("     |                                                                       |\n");
    printf("     |  ENTER CUSTOMER NAME : ");     scanf(" %[^\n]", search);
    printf("     |_______________________________________________________________________|\n");
    view_details(search);
    int option=admin_option();
    switch (option) {
        case 1: create_acc(); break;
        case 2: customer_details(); break;
        case 3: delete_acc(); break;
        case 4: logout(); break;
        default: break;
    }
}

void delete_acc(){
    int acc_no;
    char pin[10];

    printf("      _______________________________________________________________________\n");
    printf("     |                                                                       |\n");
    printf("     |                        ACCOUNT DELETION PANEL                         |\n");
    printf("     |_______________________________________________________________________|\n");
    printf("     |       ACCOUNT DELETION TO BE DONE IN PRESENCE OF ACCOUNT HOLDER       |\n");
    printf("     |                                                                       |\n");
    printf("     |  ENTER ACCOUNT NUMBER : ");     scanf("%d",&acc_no);
    printf("     |  ENTER PIN            : ");     get_pin(pin, sizeof(pin));
    printf("     |_______________________________________________________________________|\n");


    // Step 1: Fetch encrypted PIN from database
    const char *sql = "SELECT pin FROM customers WHERE account_no = ?;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, acc_no);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char *db_pin_text = sqlite3_column_text(stmt, 0);
        int stored_enc = atoi((const char*)db_pin_text);
        int stored_pin = stored_enc >> 3; // decrypt

        sqlite3_finalize(stmt);

        int entered_int = atoi(pin);
        if (entered_int == stored_pin) {
            // Step 2: PIN verified → delete account
            const char *del_sql = "DELETE FROM customers WHERE account_no = ?;";
            rc = sqlite3_prepare_v2(db, del_sql, -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Failed to prepare delete statement: %s\n", sqlite3_errmsg(db));
                return;
            }

            sqlite3_bind_int(stmt, 1, acc_no);

            if (sqlite3_step(stmt) == SQLITE_DONE) {
                printf("     |   ACCOUNT DELETED SUCCESSFULLY                                       |\n");
            } else {
                printf("     |   Failed to delete account.                                          |\n");
            }

            sqlite3_finalize(stmt);
        } else {
            printf("     |   Incorrect PIN. Account deletion aborted.                    |\n");
        }
    } else {
        printf("     |   No account found with number %d.              |\n", acc_no);
        sqlite3_finalize(stmt);
    }
    int option=admin_option();
    switch (option) {
        case 1: create_acc(); break;
        case 2: customer_details(); break;
        case 3: delete_acc(); break;
        case 4: logout(); break;
        default: break;
    }
}

void logout(){
	choice();
	return;
}

int main() {
    // Open database named bankdb.db
    const char* filePath = "D:\\Ryan\\projects\\BankMate\\bankdb.db";
    int rc = sqlite3_open(filePath, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    printf("Connection successful\n");

    while (1) {
        int option = choice();
        switch (option) {
            case 1: admin(); break;
            case 2: customers(); break;
            case 3: sqlite3_close(db); return 0;
            default: break;
        }
    }
    return 0;
}
