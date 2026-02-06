#include <stdio.h>
#include <string.h>

// ===================== Verhoeff Tables =====================

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

// ===================== Verhoeff Functions =====================

// Returns checksum value (0 = valid)
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

// ===================== Main Program =====================
int main() {
    char aadhaar[20];

    printf("Enter Aadhaar number (12 digits): ");
    scanf("%s", aadhaar);

    if (validate_aadhaar(aadhaar)) {
        printf("Aadhaar number is VALID\n");
    } else {
        printf("Aadhaar number is INVALID\n");
    }

    return 0;
}
