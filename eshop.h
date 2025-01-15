#define NUM_PRODUCTS 20
#define NUM_CUSTOMERS 5
#define ORDERS_PER_CUSTOMER 10

// Δομή για τα προϊόντα
typedef struct {
    char description[50];
    float price;
    int item_count;
    int order_requests;
    int sold_count;
    char failed_customers[100][50];
    int failed_count;
} Product;

void initialize_catalog(Product catalog[]);
void process_order(Product catalog[], int product_id, char *customer_name, int write_fd);
void generate_report(Product catalog[]);
