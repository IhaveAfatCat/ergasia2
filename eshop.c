#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "eshop.h"

// Αρχικοποίηση καταλόγου προϊόντων
void initialize_catalog(Product catalog[]) {
    for (int i = 0; i < NUM_PRODUCTS; i++) {
        sprintf(catalog[i].description, "Product_%d", i);
        catalog[i].price = (float)(rand() % 100 + 1); // Τιμή από 1 έως 100
        catalog[i].item_count = 2; // Διαθέσιμα 2 τεμάχια
        catalog[i].order_requests = 0;
        catalog[i].sold_count = 0;
        catalog[i].failed_count = 0;
    }
}

// Συνάρτηση διαχείρισης παραγγελιών από το eshop
void process_order(Product catalog[], int product_id, char *customer_name, int write_fd) {
    char response[100];
    catalog[product_id].order_requests++;
    if (catalog[product_id].item_count > 0) {
        catalog[product_id].item_count--;
        catalog[product_id].sold_count++;
        sprintf(response, "Order successful for %s. Total: %.2f", 
                catalog[product_id].description, catalog[product_id].price);
    } else {
        strcpy(catalog[product_id].failed_customers[catalog[product_id].failed_count++], customer_name);
        sprintf(response, "Order failed for %s. Product out of stock.", catalog[product_id].description);
    }
    write(write_fd, response, strlen(response) + 1);
    sleep(1); // Προσομοίωση χρόνου διεκπεραίωσης
}

// Αναφορά αποτελεσμάτων
void generate_report(Product catalog[]) {
    printf("\n--- E-Shop Report ---\n");
    int total_orders = 0, successful_orders = 0, failed_orders = 0;
    float total_revenue = 0.0;

    for (int i = 0; i < NUM_PRODUCTS; i++) {
        printf("\nProduct: %s\n", catalog[i].description);
        printf("  Requests: %d\n", catalog[i].order_requests);
        printf("  Sold: %d\n", catalog[i].sold_count);
        printf("  Failed Customers: ");
        for (int j = 0; j < catalog[i].failed_count; j++) {
            printf("%s ", catalog[i].failed_customers[j]);
        }
        printf("\n");

        total_orders += catalog[i].order_requests;
        successful_orders += catalog[i].sold_count;
        failed_orders += catalog[i].failed_count;
        total_revenue += catalog[i].sold_count * catalog[i].price;
    }

    printf("\n--- Summary ---\n");
    printf("Total Orders: %d\n", total_orders);
    printf("Successful Orders: %d\n", successful_orders);
    printf("Failed Orders: %d\n", failed_orders);
    printf("Total Revenue: %.2f\n", total_revenue);
}

int main() {
    Product catalog[NUM_PRODUCTS];
    initialize_catalog(catalog);

    int customer_to_shop[NUM_CUSTOMERS][2];
    int shop_to_customer[NUM_CUSTOMERS][2];
    pid_t pids[NUM_CUSTOMERS];

    // Δημιουργία διοχετεύσεων για κάθε πελάτη
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pipe(customer_to_shop[i]);
        pipe(shop_to_customer[i]);
    }

    srand(time(NULL)); // Τυχαίοι αριθμοί για τις παραγγελίες

    // Δημιουργία διεργασιών πελατών
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        if ((pids[i] = fork()) == 0) {
            // Κώδικας πελάτη
            char customer_name[50];
            sprintf(customer_name, "Customer_%d", i);

            close(customer_to_shop[i][0]);
            close(shop_to_customer[i][1]);
            srand(time(NULL) ^ (getpid() << 16));

            for (int j = 0; j < ORDERS_PER_CUSTOMER; j++) {
                int product_id = rand() % NUM_PRODUCTS;
                write(customer_to_shop[i][1], &product_id, sizeof(int));

                char response[100];
                read(shop_to_customer[i][0], response, sizeof(response));
                printf("[%s] %s\n", customer_name, response);

                sleep(1); // Αναμονή πριν την επόμενη παραγγελία
            }
            exit(0);
        }
    }

    // Κώδικας e-shop
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        close(customer_to_shop[i][1]);
        close(shop_to_customer[i][0]);
    }

    // Επεξεργασία παραγγελιών από το e-shop
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        for (int j = 0; j < ORDERS_PER_CUSTOMER; j++) {
            int product_id;
            // Ανάγνωση αναγνωριστικού προϊόντος από τον πελάτη μέσω του pipe
            read(customer_to_shop[i][0], &product_id, sizeof(int));

            // Δημιουργία του σωστού ονόματος πελάτη
            char customer_name[50];
            sprintf(customer_name, "Customer_%d", i);

            // Επεξεργασία της παραγγελίας με το πραγματικό όνομα του πελάτη
            process_order(catalog, product_id, customer_name, shop_to_customer[i][1]);
        }
    }

    // Αναμονή για ολοκλήρωση πελατών
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        wait(NULL);
    }

    // Γενική αναφορά
    generate_report(catalog);

    return 0;
}
