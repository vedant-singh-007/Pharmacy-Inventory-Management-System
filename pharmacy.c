#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ORDER 4 // Order of the B-tree
#define MIN_KEYS ((ORDER - 1) / 2)
#define MAX_SUPPLIERS 400    // Maximum suppliers in a batch
#define MAX_BATCHES 100      // Maximum batches per medication
#define MAX_MEDICATIONS 1000 // Maximum medications
#define MAX_NAME_LENGTH 50   // Maximum length for names
#define MAX_MEDS_PER_SUPP 100
#define MAX_DATE_LENGTH 11

typedef enum
{
    FAILURE,
    SUCCESS
} status_code;

typedef enum
{
    false,
    true
} Boolean;

int suppliers_active[MAX_SUPPLIERS] = {0}; // Array to track active suppliers

// Forward declaration
typedef struct data_tag data;

// Supplier structure holds supplier details.
typedef struct Supplier_tag
{
    int supplier_id;
    char supp_name[MAX_NAME_LENGTH];
    int qty_of_supply;
    long long contact;
    data *medications[MAX_MEDS_PER_SUPP]; // Pointers to medication data
    int med_count;
} supplier;

// Batch structure holds information about one batch of a medication.
typedef struct batch_tag
{
    int batch_no;
    int qty_instock;
    char exp_date[MAX_DATE_LENGTH];
    supplier *suppliers[MAX_SUPPLIERS]; // Array of pointers to suppliers for this batch
    int supplier_count;                 // Number of suppliers in this batch
} batch;

// Medication data structure holds medID, medname, an array of batches, and reorder level.
typedef struct data_tag
{
    int medID;
    char medname[MAX_NAME_LENGTH];
    batch Batch[MAX_BATCHES]; // Array of batch information
    int batch_count;          // Number of batches for this medication
    int reorder_lvl;
    int priceperunit;
} data;

// B-tree node for medication ID (integer key)
typedef struct B
{
    int keys[ORDER - 1];       // Integer keys (medID)
    data *values[ORDER - 1];   // Pointer to medication record
    struct B *children[ORDER]; // Child pointers
    int num_keys;              // Number of keys in the node
    int is_leaf;               // 1 if leaf, 0 otherwise
} B;

// B-tree node for medication name (string key)
typedef struct B_str
{
    char *keys[ORDER - 1];         // String keys (medname or expiry date)
    data *values[ORDER - 1];       // Pointer to medication record
    struct B_str *children[ORDER]; // Child pointers
    int num_keys;                  // Number of keys
    int is_leaf;                   // 1 if leaf, 0 otherwise
} B_str;

// B-tree node for supplier (integer key)
typedef struct B_supp
{
    int keys[ORDER - 1];            // supplier_id keys
    supplier *values[ORDER - 1];    // Pointer to supplier data
    struct B_supp *children[ORDER]; // Child pointers
    int num_keys;
    int is_leaf;
} B_supp;

supplier *search_supplier(B_supp *root, int supplier_id);
void addMedicationGeneral(B **medIDTree, B_str **mednameTree, B_str **expDateTree, B_supp **supplierTree);
void searchMedicationBymedId(int medId, B *medIDTree);
void searchMedicationBymedname(char *medname, B_str *mednameTree);
void searchMedication();
void updateMedication(int medID);
B *precedingNode(B *node, int index);
B *successiveNode(B *node, int index);
B *borrowFromPrev(B *node, int index);
B *borrowFromNext(B *node, int index);
B *mergeNodes(B *node, int index);
B *deleteFromBTree(B *root, int key);
B_str *mergeBTreeStr(B_str *node, int index);
B_str *deleteFromBTreeStr(B_str *node, char *key);
B_supp *deleteFromBTreeSupp(B_supp *root, int key);
data *search_medID(B *root, int medID);
void deleteMedication(B **root, B_str **nameRoot, B_str **dateRoot, B_supp **supplierRoot, int medID, int batch_no);
void checkexpirydate(B_str *expDateTree, char *currentdate);
void stockAlerts(B *root);
void sortMedicationByExpiry(B_str *node, const char *date1, const char *date2);
void salesTracking(B *node, int medID, int qtySold);
void supplierManagement();

//==========HEAP SORT FUNCTION==========//

void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}
int left(int i)
{
    return (2 * i + 1);
}
int right(int i)
{
    return (2 * i + 2);
}
int parent(int i)
{
    return (i - 1) / 2;
}
void MaxHeapify(int A[], int ID[], int n, int i)
{
    int largest = i;
    int l = left(i);
    int r = right(i);

    if (l < n && A[l] > A[largest])
        largest = l;

    if (r < n && A[r] > A[largest])
        largest = r;

    if (largest != i)
    {
        swap(&A[i], &A[largest]);
        swap(&ID[i], &ID[largest]);
        MaxHeapify(A, ID, n, largest);
    }
}

void BuildMaxHeap(int A[], int ID[], int n)
{
    for (int i = n / 2 - 1; i >= 0; i--)
        MaxHeapify(A, ID, n, i);
}

void Heapsort(int A[], int ID[], int n)
{
    BuildMaxHeap(A, ID, n);
    for (int i = n - 1; i > 0; i--)
    {
        swap(&A[0], &A[i]);
        swap(&ID[0], &ID[i]);
        MaxHeapify(A, ID, i, 0);
    }
}
void swap_batches(batch *a, batch *b)
{
    batch temp = *a;
    *a = *b;
    *b = temp;
}
void MaxHeapify_batches(batch *batches, int n, int i)
{
    int largest = i;
    int l = left(i);
    int r = right(i);

    if (l < n && strcmp(batches[l].exp_date, batches[largest].exp_date) > 0)
        largest = l;

    if (r < n && strcmp(batches[r].exp_date, batches[largest].exp_date) > 0)
        largest = r;

    if (largest != i)
    {
        swap_batches(&batches[i], &batches[largest]);
        MaxHeapify_batches(batches, n, largest);
    }
}

void BuildMaxHeap_batches(batch *batches, int n)
{
    for (int i = n / 2 - 1; i >= 0; i--)
        MaxHeapify_batches(batches, n, i);
}

void Heapsort_batches(batch *batches, int n)
{
    BuildMaxHeap_batches(batches, n);
    for (int i = n - 1; i > 0; i--)
    {
        swap_batches(&batches[0], &batches[i]);
        MaxHeapify_batches(batches, i, 0); // Reduce heap size by 1
    }
}

void sortBatchesByExpiry(batch *batches, int batch_count)
{
    Heapsort_batches(batches, batch_count);
}

void convert_date_format(const char *input_date, char *output_date)
{
    // Assuming input_date is in the format "DD-MM-YYYY"
    // output_date should be a buffer of at least 11 bytes
    char day[3], month[3], year[5];

    // Extract parts using sscanf
    sscanf(input_date, "%2s-%2s-%4s", day, month, year);

    // Rearrange into YYYY-MM-DD
    sprintf(output_date, "%s-%s-%s", year, month, day);
}

void reverse_date_format(const char *input_date, char *output_date)
{
    // Assuming input_date is in the format "YYYY-MM-DD"
    // output_date should be a buffer of at least 11 bytes
    char day[3], month[3], year[5];

    // Extract parts using sscanf
    sscanf(input_date, "%4s-%2s-%2s", year, month, day);

    // Rearrange into DD-MM-YYYY
    sprintf(output_date, "%s-%s-%s", day, month, year);
}

B *create_node()
{
    B *new_node = (B *)malloc(sizeof(B));
    new_node->num_keys = 0;
    new_node->is_leaf = 1;
    for (int i = 0; i < ORDER; i++)
        new_node->children[i] = NULL;
    return new_node;
}

void split_child(B *parent, int index, B *child)
{
    B *new_child = create_node();
    new_child->is_leaf = child->is_leaf;
    new_child->num_keys = (ORDER / 2) - 1;

    for (int i = 0; i < (ORDER / 2) - 1; i++)
    {
        new_child->keys[i] = child->keys[i + (ORDER / 2)];
        new_child->values[i] = child->values[i + (ORDER / 2)];
    }

    if (!child->is_leaf)
    {
        for (int i = 0; i < ORDER / 2; i++)
            new_child->children[i] = child->children[i + (ORDER / 2)];
    }

    child->num_keys = (ORDER / 2) - 1;

    for (int i = parent->num_keys; i >= index + 1; i--)
        parent->children[i + 1] = parent->children[i];
    parent->children[index + 1] = new_child;

    for (int i = parent->num_keys - 1; i >= index; i--)
    {
        parent->keys[i + 1] = parent->keys[i];
        parent->values[i + 1] = parent->values[i];
    }

    parent->keys[index] = child->keys[(ORDER / 2) - 1];
    parent->values[index] = child->values[(ORDER / 2) - 1];
    parent->num_keys++;
}

void insert_non_full(B *node, int key, data *value)
{
    int i = node->num_keys - 1;

    if (node->is_leaf)
    {
        while (i >= 0 && key < node->keys[i])
        {
            node->keys[i + 1] = node->keys[i];
            node->values[i + 1] = node->values[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->values[i + 1] = value;
        node->num_keys++;
    }
    else
    {
        while (i >= 0 && key < node->keys[i])
            i--;
        i++;
        if (node->children[i]->num_keys == ORDER - 1)
        {
            split_child(node, i, node->children[i]);
            if (key > node->keys[i])
                i++;
        }
        insert_non_full(node->children[i], key, value);
    }
}
void insert_int(B **root, int key, data *value)
{
    if (*root == NULL)
    {
        *root = create_node();
        (*root)->keys[0] = key;
        (*root)->values[0] = value;
        (*root)->num_keys = 1;
    }
    else
    {
        if ((*root)->num_keys == ORDER - 1)
        {
            B *new_root = create_node();
            new_root->is_leaf = 0;
            new_root->children[0] = *root;
            split_child(new_root, 0, *root);
            *root = new_root;
        }
        insert_non_full(*root, key, value);
    }
}

B *medIDTree = NULL;
B_str *mednameTree = NULL;
B_str *expDateTree = NULL;
B_supp *supplierTree = NULL;

B_str *create_node_str()
{
    B_str *new_node = (B_str *)malloc(sizeof(B_str));
    new_node->num_keys = 0;
    new_node->is_leaf = 1;
    for (int i = 0; i < ORDER; i++)
        new_node->children[i] = NULL;
    return new_node;
}

void split_child_str(B_str *parent, int index, B_str *child)
{
    B_str *new_child = create_node_str();
    new_child->is_leaf = child->is_leaf;
    new_child->num_keys = (ORDER / 2) - 1;

    for (int i = 0; i < (ORDER / 2) - 1; i++)
    {
        new_child->keys[i] = child->keys[i + (ORDER / 2)];
        new_child->values[i] = child->values[i + (ORDER / 2)];
    }

    if (!child->is_leaf)
    {
        for (int i = 0; i < ORDER / 2; i++)
            new_child->children[i] = child->children[i + (ORDER / 2)];
    }

    child->num_keys = (ORDER / 2) - 1;

    for (int i = parent->num_keys; i >= index + 1; i--)
        parent->children[i + 1] = parent->children[i];
    parent->children[index + 1] = new_child;

    for (int i = parent->num_keys - 1; i >= index; i--)
    {
        parent->keys[i + 1] = parent->keys[i];
        parent->values[i + 1] = parent->values[i];
    }

    parent->keys[index] = child->keys[(ORDER / 2) - 1];
    parent->values[index] = child->values[(ORDER / 2) - 1];
    parent->num_keys++;
}

void insert_non_full_str(B_str *node, char *key, data *value)
{
    int i = node->num_keys - 1;
    if (node->is_leaf)
    {
        while (i >= 0 && strcmp(key, node->keys[i]) < 0)
        {
            node->keys[i + 1] = node->keys[i];
            node->values[i + 1] = node->values[i];
            i--;
        }
        node->keys[i + 1] = strdup(key);
        node->values[i + 1] = value;
        node->num_keys++;
    }
    else
    {
        while (i >= 0 && strcmp(key, node->keys[i]) < 0)
            i--;
        i++;
        if (node->children[i]->num_keys == ORDER - 1)
        {
            split_child_str(node, i, node->children[i]);
            if (strcmp(key, node->keys[i]) > 0)
                i++;
        }
        insert_non_full_str(node->children[i], key, value);
    }
}

void insert_str(B_str **root, char *key, data *value)
{
    if (*root == NULL)
    {
        *root = create_node_str();
        (*root)->keys[0] = strdup(key);
        (*root)->values[0] = value;
        (*root)->num_keys = 1;
    }
    else
    {
        if ((*root)->num_keys == ORDER - 1)
        {
            B_str *new_root = create_node_str();
            new_root->is_leaf = 0;
            new_root->children[0] = *root;
            split_child_str(new_root, 0, *root);
            *root = new_root;
        }
        insert_non_full_str(*root, key, value);
    }
}

B_supp *create_node_supp()
{
    B_supp *new_node = (B_supp *)malloc(sizeof(B_supp));
    new_node->num_keys = 0;
    new_node->is_leaf = 1;
    for (int i = 0; i < ORDER; i++)
        new_node->children[i] = NULL;
    return new_node;
}

void split_child_supp(B_supp *parent, int index, B_supp *child)
{
    B_supp *new_child = create_node_supp();
    new_child->is_leaf = child->is_leaf;
    new_child->num_keys = (ORDER / 2) - 1;

    for (int i = 0; i < (ORDER / 2) - 1; i++)
    {
        new_child->keys[i] = child->keys[i + (ORDER / 2)];
        new_child->values[i] = child->values[i + (ORDER / 2)];
    }

    if (!child->is_leaf)
    {
        for (int i = 0; i < ORDER / 2; i++)
            new_child->children[i] = child->children[i + (ORDER / 2)];
    }

    child->num_keys = (ORDER / 2) - 1;

    for (int i = parent->num_keys; i >= index + 1; i--)
        parent->children[i + 1] = parent->children[i];
    parent->children[index + 1] = new_child;

    for (int i = parent->num_keys - 1; i >= index; i--)
    {
        parent->keys[i + 1] = parent->keys[i];
        parent->values[i + 1] = parent->values[i];
    }

    parent->keys[index] = child->keys[(ORDER / 2) - 1];
    parent->values[index] = child->values[(ORDER / 2) - 1];
    parent->num_keys++;
}

void insert_non_full_supp(B_supp *node, int key, supplier *value)
{
    int i = node->num_keys - 1;
    if (node->is_leaf)
    {
        while (i >= 0 && key < node->keys[i])
        {
            node->keys[i + 1] = node->keys[i];
            node->values[i + 1] = node->values[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->values[i + 1] = value;
        node->num_keys++;
    }
    else
    {
        while (i >= 0 && key < node->keys[i])
            i--;
        i++;
        if (node->children[i]->num_keys == ORDER - 1)
        {
            split_child_supp(node, i, node->children[i]);
            if (key > node->keys[i])
                i++;
        }
        insert_non_full_supp(node->children[i], key, value);
    }
}

void insert_supp(B_supp **root, int key, supplier *value)
{
    supplier *new_supplier = (supplier *)malloc(sizeof(supplier));
    if (new_supplier == NULL)
    {
        printf("Memory allocation failed for supplier.\n");
        return;
    }
    *new_supplier = *value; // Copy supplier details to prevent overwriting

    if (*root == NULL)
    {
        *root = create_node_supp();
        (*root)->keys[0] = key;
        (*root)->values[0] = new_supplier; // Store new supplier pointer
        (*root)->num_keys = 1;
    }
    else
    {
        if ((*root)->num_keys == ORDER - 1)
        {
            B_supp *new_root = create_node_supp();
            new_root->is_leaf = 0;
            new_root->children[0] = *root;
            split_child_supp(new_root, 0, *root);
            *root = new_root;
        }
        insert_non_full_supp(*root, key, new_supplier);
    }
}
supplier *search_supplier(B_supp *root, int supplier_id)
{
    if (root == NULL)
        return NULL;

    int i = 0;

    // Find the first key greater than or equal to supplier_id
    while (i < root->num_keys && supplier_id > root->keys[i])
        i++;

    // If the key is found, return the associated supplier data
    if (i < root->num_keys && root->keys[i] == supplier_id)
        return root->values[i];

    // If the node is a leaf, the key doesn't exist
    if (root->is_leaf)
        return NULL;

    // Search in the appropriate child node
    return search_supplier(root->children[i], supplier_id);
}
void addMedicationGeneral(B **medIDTree, B_str **mednameTree, B_str **expDateTree, B_supp **supplierTree)
{
    // Input medication data
    int medID, batch_no, priceperunit, qty_instock, reorder_lvl;
    char medname[MAX_NAME_LENGTH];
    char exp_date[MAX_DATE_LENGTH];

    printf("\n=== Add New Medication ===\n");
    printf("Enter Medication ID: ");
    scanf("%d", &medID);
    printf("Enter Medication Name: ");
    scanf(" %[^\n]", medname);
    printf("Enter Batch Number: ");
    scanf("%d", &batch_no);
    printf("Enter Price per Unit: ");
    scanf("%d", &priceperunit);
    printf("Enter Quantity in Stock: ");
    scanf("%d", &qty_instock);
    printf("Enter Expiry Date (YYYY-MM-DD): ");
    scanf(" %10s", exp_date);
    printf("Enter Reorder Level: ");
    scanf("%d", &reorder_lvl);

    // Medication search or create
    data *med = search_medID(*medIDTree, medID);
    if (med == NULL)
    {
        med = (data *)malloc(sizeof(data));
        if (!med)
        {
            printf("Memory allocation failed for medication\n");
            return;
        }

        // Initialize medication
        med->medID = medID;
        strncpy(med->medname, medname, MAX_NAME_LENGTH);
        med->medname[MAX_NAME_LENGTH - 1] = '\0';
        med->reorder_lvl = reorder_lvl;
        med->priceperunit = priceperunit;
        med->batch_count = 0;

        insert_int(medIDTree, med->medID, med);
        insert_str(mednameTree, med->medname, med);
    }

    // Check batch limit
    if (med->batch_count >= MAX_BATCHES)
    {
        printf("Error: Maximum batches (%d) reached for this medication\n", MAX_BATCHES);
        return;
    }

    // Find insertion position for batch (sorted by batch_no)
    int pos = 0;
    while (pos < med->batch_count && batch_no > med->Batch[pos].batch_no)
    {
        pos++;
    }

    // Shift batches to make room
    for (int i = med->batch_count; i > pos; i--)
    {
        med->Batch[i] = med->Batch[i - 1];
    }

    // Initialize new batch
    batch *newBatch = &med->Batch[pos];
    newBatch->batch_no = batch_no;
    newBatch->qty_instock = qty_instock;
    strncpy(newBatch->exp_date, exp_date, MAX_DATE_LENGTH);
    newBatch->exp_date[MAX_DATE_LENGTH - 1] = '\0';
    newBatch->supplier_count = 0;
    med->batch_count++;
    insert_str(expDateTree, newBatch->exp_date, med);

    // Supplier handling
    int supplier_id;
    printf("\n=== Supplier Information ===\n");
    printf("Enter Supplier ID: ");
    scanf("%d", &supplier_id);

    supplier *existingSupplier = search_supplier(*supplierTree, supplier_id);
    if (existingSupplier == NULL)
    {
        // New supplier
        existingSupplier = (supplier *)malloc(sizeof(supplier));
        if (!existingSupplier)
        {
            printf("Memory allocation failed for supplier\n");
            return;
        }

        existingSupplier->supplier_id = supplier_id;
        printf("Enter Supplier Name: ");
        scanf(" %[^\n]", existingSupplier->supp_name);
        printf("Enter Quantity Supplied: ");
        scanf("%d", &existingSupplier->qty_of_supply);
        printf("Enter Supplier Contact: ");
        scanf("%lld", &existingSupplier->contact);
        existingSupplier->med_count = 0;

        insert_supp(supplierTree, existingSupplier->supplier_id, existingSupplier);
    }

    // Link supplier to batch
    if (newBatch->supplier_count < MAX_SUPPLIERS)
    {
        newBatch->suppliers[newBatch->supplier_count++] = existingSupplier;
        printf("Supplier linked to batch successfully.\n");
    }
    else
    {
        printf("Warning: Maximum suppliers (%d) reached for this batch\n", MAX_SUPPLIERS);
    }

    // Link medication to supplier if not already linked
    Boolean medExists = false;
    for (int i = 0; i < existingSupplier->med_count; i++)
    {
        if (existingSupplier->medications[i] &&
            existingSupplier->medications[i]->medID == medID)
        {
            medExists = true;
            break;
        }
    }

    if (!medExists && existingSupplier->med_count < MAX_MEDS_PER_SUPP)
    {
        existingSupplier->medications[existingSupplier->med_count++] = med;
        printf("Medication linked to supplier successfully.\n");
    }
}

void inorderBTree(B *root)
{
    if (root != NULL)
    {
        for (int i = 0; i < root->num_keys; i++)
        {
            inorderBTree(root->children[i]);
            printf("%d ", root->keys[i]); // Print medID
        }
        inorderBTree(root->children[root->num_keys]);
    }
}

void inorderBTreeStr(B_str *root)
{
    if (root != NULL)
    {
        for (int i = 0; i < root->num_keys; i++)
        {
            inorderBTreeStr(root->children[i]);
            printf("%s ", root->keys[i]); // Print medname or exp_date
        }
        inorderBTreeStr(root->children[root->num_keys]);
    }
}
void inorderBTreeExp(B_str *root)
{
    if (root != NULL)
    {
        for (int i = 0; i < root->num_keys; i++)
        {
            inorderBTreeExp(root->children[i]);

            // Convert and print the date in DD-MM-YYYY format
            char formatted_date[11];
            reverse_date_format(root->keys[i], formatted_date);
            printf("%s ", formatted_date);
        }
        inorderBTreeExp(root->children[root->num_keys]);
    }
}

void inorderBTreeSupp(B_supp *root)
{
    if (root != NULL)
    {
        for (int i = 0; i < root->num_keys; i++)
        {
            // Traverse left child first
            inorderBTreeSupp(root->children[i]);

            // Print supplier details
            printf("Supplier ID: %d\n", root->keys[i]);
            printf("Supplier Name: %s\n", root->values[i]->supp_name);
            printf("Quantity of Supply: %d\n", root->values[i]->qty_of_supply);
            printf("Contact: %lld\n", root->values[i]->contact);
            printf("Medications Supplied:\n");
            if (root->values[i]->med_count > 0)
            {

                for (int j = 0; j < root->values[i]->med_count; j++)
                {
                    if (root->values[i]->medications[j] != NULL)
                    {
                        printf("  - %s (ID: %d)\n",
                               root->values[i]->medications[j]->medname,
                               root->values[i]->medications[j]->medID);
                    }
                }
            }
            else
            {
                printf("  (No medications recorded)\n");
            }
            printf("\n");
        }
        // Traverse rightmost child
        inorderBTreeSupp(root->children[root->num_keys]);
    }
}
void searchMedicationBymedId(int medId, B *medIDTree)
{
    B *current = medIDTree;
    while (current != NULL)
    {
        int i = 0;
        while (i < current->num_keys && medId > current->keys[i])
        {
            i++;
        }

        if (i < current->num_keys && medId == current->keys[i])
        {
            data *med = current->values[i];
            printf("\nMedication Details:\n");
            printf("ID: %d\n", med->medID);
            printf("Name: %s\n", med->medname);
            printf("Price per unit: %d\n", med->priceperunit);
            printf("Reorder level: %d\n", med->reorder_lvl);
            printf("Batch Count: %d\n", med->batch_count);

            for (int j = 0; j < med->batch_count; j++)
            {
                printf("\nBatch %d:\n", j + 1);
                printf("  Batch No: %d\n", med->Batch[j].batch_no);
                printf("  Expiry Date: %s\n", med->Batch[j].exp_date);
                printf("  Quantity in Stock: %d\n", med->Batch[j].qty_instock);
                printf("  Suppliers: ");

                if (med->Batch[j].supplier_count == 0)
                {
                    printf("None\n");
                }
                else
                {
                    for (int k = 0; k < med->Batch[j].supplier_count; k++)
                    {
                        if (med->Batch[j].suppliers[k])
                        {
                            printf("%s (ID: %d)",
                                   med->Batch[j].suppliers[k]->supp_name,
                                   med->Batch[j].suppliers[k]->supplier_id);
                            if (k < med->Batch[j].supplier_count - 1)
                                printf(", ");
                        }
                    }
                    printf("\n");
                }
            }
            return;
        }

        if (current->is_leaf)
        {
            break;
        }
        else
        {
            current = current->children[i];
        }
    }
    printf("Medication with ID %d not found.\n", medId);
}

void searchMedicationBymedname(char *medname, B_str *mednameTree)
{
    B_str *current = mednameTree;
    while (current != NULL)
    {
        int i = 0;
        while (i < current->num_keys && strcmp(medname, current->keys[i]) > 0)
        {
            i++;
        }
        if (i < current->num_keys && strcmp(medname, current->keys[i]) == 0)
        {
            printf("Medication found: Name %s\n", medname);
            printf("ID: %d\n", current->values[i]->medID);
            printf("Batch Count: %d\n", current->values[i]->batch_count);
            for (int j = 0; j < current->values[i]->batch_count; j++)
            {
                printf("Batch No: %d, Expiry Date: %s, Quantity in Stock: %d\n",
                       current->values[i]->Batch[j].batch_no,
                       current->values[i]->Batch[j].exp_date,
                       current->values[i]->Batch[j].qty_instock);
            }
            printf("Reorder Level: %d\n", current->values[i]->reorder_lvl);
            return;
        }
        if (current->is_leaf)
        {
            break;
        }
        else
        {
            current = current->children[i];
        }
    }
    printf("Medication with name %s not found.\n", medname);
}
void updateMedication(int medID)
{
    // Search for existing medication by medID.
    data *med = search_medID(medIDTree, medID);
    if (med == NULL)
    {
        printf("Medication with ID %d not found.\n", medID);
        return;
    }
    else
    {
        printf("enter the new price of medication\n");
        int new_price;
        scanf("%d", &new_price);
        med->priceperunit = new_price;
        printf("enter the batch number whose quantity is to be updated");
        int batch_no;
        scanf("%d", &batch_no);
        int i;
        for (i = 0; i < med->batch_count; i++)
        {
            if (med->Batch[i].batch_no == batch_no)
            {
                break;
            }
        }
        if (i == med->batch_count)
        {
            printf("Batch number %d not found for medication ID %d.\n", batch_no, medID);
            return;
        }
        printf("enter the new quantity of medication\n");
        int new_qty;
        scanf("%d", &new_qty);
        med->Batch[i].qty_instock = new_qty;
        printf("Batch number %d updated successfully for medication ID %d.\n", batch_no, medID);
    }
}

B *precedingNode(B *node, int index)
{
    B *current = node->children[index];
    while (current->children[0] != NULL)
        current = current->children[current->num_keys];
    return current;
}

B *successiveNode(B *node, int index)
{
    B *current = node->children[index + 1];
    while (current->children[0] != NULL)
        current = current->children[0];
    return current;
}

B *borrowFromPrev(B *node, int index)
{
    B *child = node->children[index];
    B *sibling = node->children[index - 1];

    for (int i = child->num_keys - 1; i >= 0; i--)
    {
        child->keys[i + 1] = child->keys[i];
        child->values[i + 1] = child->values[i];
    }

    if (!child->is_leaf)
    {
        for (int i = child->num_keys; i >= 0; i--)
            child->children[i + 1] = child->children[i];
    }

    child->keys[0] = node->keys[index - 1];
    child->values[0] = node->values[index - 1];

    if (!child->is_leaf)
        child->children[0] = sibling->children[sibling->num_keys];

    node->keys[index - 1] = sibling->keys[sibling->num_keys - 1];
    node->values[index - 1] = sibling->values[sibling->num_keys - 1];

    child->num_keys++;
    sibling->num_keys--;

    return node;
}

B *borrowFromNext(B *node, int index)
{
    B *child = node->children[index];
    B *sibling = node->children[index + 1];

    child->keys[child->num_keys] = node->keys[index];
    child->values[child->num_keys] = node->values[index];

    if (!child->is_leaf)
        child->children[child->num_keys + 1] = sibling->children[0];

    node->keys[index] = sibling->keys[0];
    node->values[index] = sibling->values[0];

    for (int i = 1; i < sibling->num_keys; i++)
    {
        sibling->keys[i - 1] = sibling->keys[i];
        sibling->values[i - 1] = sibling->values[i];
    }

    if (!sibling->is_leaf)
    {
        for (int i = 1; i <= sibling->num_keys; i++)
            sibling->children[i - 1] = sibling->children[i];
    }

    child->num_keys++;
    sibling->num_keys--;

    return node;
}

B *mergeNodes(B *node, int index)
{
    B *child = node->children[index];
    B *sibling = node->children[index + 1];

    child->keys[MIN_KEYS] = node->keys[index];
    child->values[MIN_KEYS] = node->values[index];

    for (int i = 0; i < sibling->num_keys; i++)
    {
        child->keys[i + MIN_KEYS + 1] = sibling->keys[i];
        child->values[i + MIN_KEYS + 1] = sibling->values[i];
    }

    if (!child->is_leaf)
    {
        for (int i = 0; i <= sibling->num_keys; i++)
            child->children[i + MIN_KEYS + 1] = sibling->children[i];
    }

    for (int i = index + 1; i < node->num_keys; i++)
    {
        node->keys[i - 1] = node->keys[i];
        node->values[i - 1] = node->values[i];
    }

    for (int i = index + 2; i <= node->num_keys; i++)
        node->children[i - 1] = node->children[i];

    child->num_keys += sibling->num_keys + 1;
    node->num_keys--;

    free(sibling);
    return node;
}

B *deleteFromBTree(B *root, int key)
{
    if (!root)
        return NULL;

    int i = 0;
    while (i < root->num_keys && key > root->keys[i])
        i++;

    if (i < root->num_keys && key == root->keys[i])
    {
        if (root->is_leaf)
        {
            for (int j = i; j < root->num_keys - 1; j++)
            {
                root->keys[j] = root->keys[j + 1];
                root->values[j] = root->values[j + 1];
            }
            root->num_keys--;
        }
        else
        {
            if (root->children[i]->num_keys >= MIN_KEYS + 1)
            {
                B *pred = precedingNode(root, i);
                root->keys[i] = pred->keys[pred->num_keys - 1];
                root->values[i] = pred->values[pred->num_keys - 1];
                root->children[i] = deleteFromBTree(root->children[i], root->keys[i]);
            }
            else if (root->children[i + 1]->num_keys >= MIN_KEYS + 1)
            {
                B *succ = successiveNode(root, i);
                root->keys[i] = succ->keys[0];
                root->values[i] = succ->values[0];
                root->children[i + 1] = deleteFromBTree(root->children[i + 1], root->keys[i]);
            }
            else
            {
                mergeNodes(root, i);
                root->children[i] = deleteFromBTree(root->children[i], key);
            }
        }
    }
    else if (!root->is_leaf)
    {
        if (root->children[i]->num_keys < MIN_KEYS + 1)
        {
            if (i > 0 && root->children[i - 1]->num_keys >= MIN_KEYS + 1)
                borrowFromPrev(root, i);
            else if (i < root->num_keys && root->children[i + 1]->num_keys >= MIN_KEYS + 1)
                borrowFromNext(root, i);
            else
            {
                if (i < root->num_keys)
                    mergeNodes(root, i);
                else
                    mergeNodes(root, i - 1);
            }
        }
        root->children[i] = deleteFromBTree(root->children[i], key);
    }

    if (root->num_keys == 0)
    {
        B *temp = root;
        root = root->children[0];
        free(temp);
    }

    return root;
}

B_str *mergeBTreeStr(B_str *node, int index)
{
    B_str *child = node->children[index];
    B_str *sibling = node->children[index + 1];

    // Move key and value from parent to child
    child->keys[child->num_keys] = node->keys[index];
    child->values[child->num_keys] = node->values[index];
    child->num_keys++;

    // Merge keys and values from sibling
    for (int i = 0; i < sibling->num_keys; i++)
    {
        child->keys[child->num_keys] = sibling->keys[i];
        child->values[child->num_keys] = sibling->values[i];
        child->num_keys++;
    }

    // Merge children
    if (!child->is_leaf)
    {
        for (int i = 0; i <= sibling->num_keys; i++)
        {
            child->children[child->num_keys - sibling->num_keys + i] = sibling->children[i];
        }
    }

    // Shift keys, values and children in parent
    for (int i = index; i < node->num_keys - 1; i++)
    {
        node->keys[i] = node->keys[i + 1];
        node->values[i] = node->values[i + 1];
    }
    for (int i = index + 1; i <= node->num_keys; i++)
    {
        node->children[i] = node->children[i + 1];
    }

    node->num_keys--;
    free(sibling);
    return node;
}
B_str *deleteFromBTreeStr(B_str *node, char *key)
{
    if (node == NULL)
        return NULL;

    int i = 0;
    while (i < node->num_keys && strcmp(key, node->keys[i]) > 0)
        i++;

    if (i < node->num_keys && strcmp(key, node->keys[i]) == 0)
    {
        if (node->is_leaf)
        {
            free(node->keys[i]);
            for (int j = i; j < node->num_keys - 1; j++)
            {
                node->keys[j] = node->keys[j + 1];
                node->values[j] = node->values[j + 1];
            }
            node->num_keys--;
        }
        else
        {
            B_str *pred = node->children[i];
            while (!pred->is_leaf)
                pred = pred->children[pred->num_keys];

            free(node->keys[i]);
            node->keys[i] = strdup(pred->keys[pred->num_keys - 1]);
            node->values[i] = pred->values[pred->num_keys - 1];

            node->children[i] = deleteFromBTreeStr(node->children[i], pred->keys[pred->num_keys - 1]);
        }
    }
    else if (!node->is_leaf)
    {
        node->children[i] = deleteFromBTreeStr(node->children[i], key);

        if (node->children[i] && node->children[i]->num_keys < (ORDER - 1) / 2)
        {
            // Borrow from previous sibling
            if (i > 0 && node->children[i - 1]->num_keys > (ORDER - 1) / 2)
            {
                B_str *child = node->children[i];
                B_str *sibling = node->children[i - 1];

                for (int j = child->num_keys; j > 0; j--)
                {
                    child->keys[j] = child->keys[j - 1];
                    child->values[j] = child->values[j - 1];
                }

                if (!child->is_leaf)
                {
                    for (int j = child->num_keys + 1; j > 0; j--)
                        child->children[j] = child->children[j - 1];
                }

                child->keys[0] = strdup(node->keys[i - 1]);
                child->values[0] = node->values[i - 1];

                if (!child->is_leaf)
                    child->children[0] = sibling->children[sibling->num_keys];

                node->keys[i - 1] = sibling->keys[sibling->num_keys - 1];
                node->values[i - 1] = sibling->values[sibling->num_keys - 1];

                sibling->num_keys--;
                child->num_keys++;
            }
            // Borrow from next sibling
            else if (i < node->num_keys && node->children[i + 1]->num_keys > (ORDER - 1) / 2)
            {
                B_str *child = node->children[i];
                B_str *sibling = node->children[i + 1];

                child->keys[child->num_keys] = strdup(node->keys[i]);
                child->values[child->num_keys] = node->values[i];

                if (!child->is_leaf)
                    child->children[child->num_keys + 1] = sibling->children[0];

                node->keys[i] = sibling->keys[0];
                node->values[i] = sibling->values[0];

                for (int j = 1; j < sibling->num_keys; j++)
                {
                    sibling->keys[j - 1] = sibling->keys[j];
                    sibling->values[j - 1] = sibling->values[j];
                }

                if (!sibling->is_leaf)
                {
                    for (int j = 1; j <= sibling->num_keys; j++)
                        sibling->children[j - 1] = sibling->children[j];
                }

                sibling->num_keys--;
                child->num_keys++;
            }
            // Merge
            else
            {
                node = mergeBTreeStr(node, i);
            }
        }
    }

    return node;
}

B_supp *deleteFromBTreeSupp(B_supp *root, int key)
{
    return (B_supp *)deleteFromBTree((B *)root, key);
}

data *search_medID(B *root, int medID)
{
    if (root == NULL)
        return NULL;

    int i = 0;
    while (i < root->num_keys && medID > root->keys[i])
        i++;

    if (i < root->num_keys && medID == root->keys[i])
        return root->values[i];

    if (root->is_leaf)
        return NULL;

    return search_medID(root->children[i], medID);
}

void deleteMedication(B **root, B_str **nameRoot, B_str **dateRoot, B_supp **supplierRoot, int medID, int batch_no)
{
    if (medID == -1)
    {
        printf("Invalid medication ID.\n");
        return;
    }

    // Search for the medication
    data *med = search_medID(*root, medID);
    if (med == NULL)
    {
        printf("Medication with ID %d not found.\n", medID);
        return;
    }

    if (batch_no == -1) // Delete entire medication
    {
        printf("Deleting medication with ID %d.\n", medID);

        // Remove from medIDTree
        *root = deleteFromBTree(*root, medID);

        // Remove from mednameTree
        *nameRoot = deleteFromBTreeStr(*nameRoot, med->medname);

        // Remove all associated batches from expDateTree
        for (int i = 0; i < med->batch_count; i++)
        {
            *dateRoot = deleteFromBTreeStr(*dateRoot, med->Batch[i].exp_date);
        }

        // Remove from supplierTree
        for (int i = 0; i < med->batch_count; i++)
        {
            for (int i = 0; i < med->batch_count; i++)
            {
                for (int j = 0; j < med->Batch[i].supplier_count; j++)
                {
                    // CORRECT: suppliers[j] is a pointer, so use ->
                    int suppID = med->Batch[i].suppliers[j]->supplier_id;
                    *supplierRoot = deleteFromBTreeSupp(*supplierRoot, suppID);
                }
            }
            free(med);
        }

        free(med);
    }
    else // Delete specific batch
    {
        int found = 0;
        for (int i = 0; i < med->batch_count; i++)
        {
            if (med->Batch[i].batch_no == batch_no)
            {

                found = 1;
                // Remove batch from expDateTree
                *dateRoot = deleteFromBTreeStr(*dateRoot, med->Batch[i].exp_date);
                // Shift remaining batches left
                for (int j = i; j < med->batch_count - 1; j++)
                {
                    med->Batch[j] = med->Batch[j + 1];
                }
                med->batch_count--;

                printf("Deleted batch %d from medication ID %d.\n", batch_no, medID);

                // If all batches are deleted, remove medication completely
                if (med->batch_count == 0)
                {
                    deleteMedication(root, nameRoot, dateRoot, supplierRoot, medID, -1);
                }
                break;
            }
        }
        if (!found)
        {
            printf("Batch number %d not found for medication ID %d.\n", batch_no, medID);
        }
    }
    printf("\n");
}

void checkexpirydate(B_str *expDateTree, char *currentdate)
{
    if (expDateTree == NULL)
    {
        printf("Tree is empty.\n");
        return;
    }

    // Convert input date (DD-MM-YYYY) to YYYY-MM-DD for comparison
    char formatted_current[11];
    convert_date_format(currentdate, formatted_current); // You already have this function

    B_str *root = expDateTree;

    for (int i = 0; i < root->num_keys; i++)
    {
        char formatted_tree_date[11];
        reverse_date_format(root->keys[i], formatted_tree_date); // For printing in DD-MM-YYYY

        if (strcmp(root->keys[i], formatted_current) < 0)
        {
            printf("Medication with ID %d and name %s has an expired batch with expiry date %s.\n",
                   root->values[i]->medID, root->values[i]->medname, formatted_tree_date);
        }
        else
        {
            int year1, month1, day1, year2, month2, day2;
            sscanf(formatted_current, "%d-%d-%d", &year1, &month1, &day1);
            sscanf(root->keys[i], "%d-%d-%d", &year2, &month2, &day2);

            int days_diff = (year2 - year1) * 365 + (month2 - month1) * 30 + (day2 - day1);
            if (days_diff <= 30 && days_diff >= 0)
            {
                printf("Medication with ID %d and name %s has a batch expiring soon (expiry date: %s).\n",
                       root->values[i]->medID, root->values[i]->medname, formatted_tree_date);
            }
        }
    }

    if (!root->is_leaf)
    {
        for (int i = 0; i <= root->num_keys; i++)
        {
            checkexpirydate(root->children[i], currentdate);
        }
    }
}

void addsupplier()
{
    printf("Enter the medication ID and batch number to be supplied: ");
    int medID, batch_no;
    scanf("%d %d", &medID, &batch_no);

    data *med = search_medID(medIDTree, medID);
    if (med == NULL)
    {
        printf("Medication with ID %d not found.\n", medID);
    }

    // Find the batch
    int i;
    for (i = 0; i < med->batch_count; i++)
    {
        if (med->Batch[i].batch_no == batch_no)
            break;
    }
    if (i == med->batch_count)
    {
        printf("Batch number %d not found for medication ID %d.\n", batch_no, medID);
    }

    printf("Enter the supplier ID: ");
    int supplier_id;
    scanf("%d", &supplier_id);

    supplier *existingSupplier = search_supplier(supplierTree, supplier_id);
    if (existingSupplier == NULL)
    {
        // New supplier
        existingSupplier = (supplier *)malloc(sizeof(supplier));
        if (!existingSupplier)
        {
            printf("Memory allocation failed for supplier.\n");
        }

        existingSupplier->supplier_id = supplier_id;
        printf("Supplier Name: ");
        scanf("%s", existingSupplier->supp_name);
        printf("Quantity of Supply: ");
        scanf("%d", &existingSupplier->qty_of_supply);
        printf("Contact: ");
        scanf("%lld", &existingSupplier->contact);

        existingSupplier->med_count = 0;
        suppliers_active[supplier_id] = 1;
    }

    // Link supplier to batch
    if (med->Batch[i].supplier_count < MAX_SUPPLIERS)
    {
        // Check if supplier already linked to this batch
        for (int j = 0; j < med->Batch[i].supplier_count; j++)
        {
            if (med->Batch[i].suppliers[j] &&
                med->Batch[i].suppliers[j]->supplier_id == supplier_id)
            {
                printf("Supplier already linked to this batch.\n");
            }
        }

        med->Batch[i].suppliers[med->Batch[i].supplier_count] = existingSupplier;
        med->Batch[i].supplier_count++;
    }
    else
    {
        printf("Maximum suppliers reached for this batch.\n");
    }

    // Link medication to supplier if not already linked
    Boolean medExists = false;
    for (int j = 0; j < existingSupplier->med_count; j++)
    {
        if (existingSupplier->medications[j] &&
            existingSupplier->medications[j]->medID == medID)
        {
            medExists = true;
            break;
        }
    }

    if (!medExists && existingSupplier->med_count < MAX_MEDS_PER_SUPP)
    {
        existingSupplier->medications[existingSupplier->med_count] = med;
        existingSupplier->med_count++;
    }

    printf("Supplier added successfully to batch %d of medication %s (ID: %d)\n",
           batch_no, med->medname, medID);
    insert_supp(&supplierTree, supplier_id, existingSupplier);
}
// Function to delete a supplier from the system
void deleteSupplier(B_supp **supplierTree)
{
    if (*supplierTree == NULL)
    {
        printf("Supplier tree is empty.\n");
        return;
    }
    int supplier_id;
    printf("Enter supplier id to be deleted: ");
    scanf("%d", &supplier_id);
    // Search for the supplier first
    supplier *suppToDelete = search_supplier(*supplierTree, supplier_id);
    if (suppToDelete == NULL)
    {
        printf("Supplier with ID %d not found.\n", supplier_id);
        return;
    }

    // Check if supplier is linked to any medications
    if (suppToDelete->med_count > 0)
    {
        printf("Warning: Supplier is linked to %d medications. Deleting anyway...\n", suppToDelete->med_count);

        // Remove supplier references from all linked medications
        for (int i = 0; i < suppToDelete->med_count; i++)
        {
            if (suppToDelete->medications[i] != NULL)
            {
                data *med = suppToDelete->medications[i];

                // Remove supplier from all batches of this medication
                for (int j = 0; j < med->batch_count; j++)
                {
                    batch *currentBatch = &med->Batch[j];

                    // Find and remove supplier from this batch
                    for (int k = 0; k < currentBatch->supplier_count; k++)
                    {
                        if (currentBatch->suppliers[k] != NULL &&
                            currentBatch->suppliers[k]->supplier_id == supplier_id)
                        {

                            // Shift remaining suppliers left
                            for (int l = k; l < currentBatch->supplier_count - 1; l++)
                            {
                                currentBatch->suppliers[l] = currentBatch->suppliers[l + 1];
                            }
                            currentBatch->supplier_count--;
                            break; // Found and removed, move to next batch
                        }
                    }
                }
            }
        }
    }

    // Now delete from the B-tree
    *supplierTree = deleteFromBTreeSupp(*supplierTree, supplier_id);

    // Update active suppliers array
    suppliers_active[supplier_id] = 0;

    printf("Supplier with ID %d deleted successfully.\n", supplier_id);
}
void collectSupplierMedCounts(B_supp *node, int *frequency, int *supplierIDs)
{
    if (node == NULL)
        return;

    for (int i = 0; i < node->num_keys; i++)
    {
        if (!node->is_leaf)
        {
            collectSupplierMedCounts(node->children[i], frequency, supplierIDs);
        }

        int supplierID = node->keys[i];
        frequency[supplierID] = node->values[i]->med_count; // Ensure med_count is updated correctly
        supplierIDs[supplierID] = supplierID;
    }

    if (!node->is_leaf)
    {
        collectSupplierMedCounts(node->children[node->num_keys], frequency, supplierIDs);
    }
}
void top10Allrounders()
{
    int frequency[500] = {0};
    int supplierIDs[500] = {0};

    collectSupplierMedCounts(supplierTree, frequency, supplierIDs);

    // Heap sort both arrays together
    Heapsort(frequency, supplierIDs, 500); // Sort in ascending order

    printf("Top 10 Suppliers based on No. of Unique Medications:\n");
    int found = 0;
    for (int i = 499; i >= 490; i--) // Get top 10 from the end (since it's ascending)
    {
        if (frequency[i] > 0)
        {
            printf("Supplier ID: %d, Unique Medications Supplied: %d\n", supplierIDs[i], frequency[i]);
            found++;
        }
    }

    if (found == 0)
    {
        printf("No suppliers have supplied any medications.\n");
    }
}

void searchbysupplier()
{
    supplier *temp;
    int sup_id;
    printf("enter supplier id whose meds u wanna know");
    scanf("%d", &sup_id);
    temp = search_supplier(supplierTree, sup_id);
    for (int i = 0; i < temp->med_count; i++)
    {
        printf("Supplies Medicine %s \n", temp->medications[i]->medname);
        printf("Med ID:%d", temp->medications[i]->medID);
        printf("Batch Count: %d\n", temp->medications[i]->batch_count);
        printf("Reorder Level: %d\n", temp->medications[i]->reorder_lvl);
        printf("Price per unit: %d\n", temp->medications[i]->priceperunit);
    }
}
// Function to update supplier contact and quantity of supply
void updateSupplier(B_supp *supplierTree)
{
    if (supplierTree == NULL)
    {
        printf("Supplier tree is empty.\n");
        return;
    }
    int supplier_id;
    printf("Enter supplier id to be updated: ");
    scanf("%d", &supplier_id);
    // Search for the supplier
    supplier *suppToUpdate = search_supplier(supplierTree, supplier_id);
    if (suppToUpdate == NULL)
    {
        printf("Supplier with ID %d not found.\n", supplier_id);
        return;
    }

    printf("\nCurrent Supplier Details:\n");
    printf("Supplier ID: %d\n", suppToUpdate->supplier_id);
    printf("1. Quantity of Supply: %d\n", suppToUpdate->qty_of_supply);
    printf("2. Contact: %lld\n", suppToUpdate->contact);

    int choice;
    printf("Choose an option to update:\n");
    printf("0. Cancel\n");
    printf("1. Update Quantity of Supply\n");
    printf("2. Update Contact\n");
    printf("3. Update Both\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    switch (choice)
    {
    case 0:
        printf("Update cancelled.\n");
        break;
    case 1: // Update quantity
        printf("Enter new quantity of supply: ");
        scanf("%d", &suppToUpdate->qty_of_supply);

        // Update quantity in all linked batches
        for (int i = 0; i < suppToUpdate->med_count; i++)
        {
            if (suppToUpdate->medications[i] != NULL)
            {
                data *med = suppToUpdate->medications[i];
                for (int j = 0; j < med->batch_count; j++)
                {
                    for (int k = 0; k < med->Batch[j].supplier_count; k++)
                    {
                        if (med->Batch[j].suppliers[k] != NULL &&
                            med->Batch[j].suppliers[k]->supplier_id == supplier_id)
                        {
                            med->Batch[j].suppliers[k]->qty_of_supply = suppToUpdate->qty_of_supply;
                        }
                    }
                }
            }
        }
        printf("Quantity of supply updated successfully.\n");
        break;
    case 2: // Update contact
        printf("Enter new contact number: ");
        scanf("%lld", &suppToUpdate->contact);
        for (int i = 0; i < suppToUpdate->med_count; i++)
        {
            if (suppToUpdate->medications[i] != NULL)
            {
                data *med = suppToUpdate->medications[i];
                for (int j = 0; j < med->batch_count; j++)
                {
                    for (int k = 0; k < med->Batch[j].supplier_count; k++)
                    {
                        if (med->Batch[j].suppliers[k] != NULL &&
                            med->Batch[j].suppliers[k]->supplier_id == supplier_id)
                        {
                            med->Batch[j].suppliers[k]->contact = suppToUpdate->contact;
                        }
                    }
                }
            }
        }
        printf("Contact number updated successfully.\n");
        break;
    case 3: // Update both
        printf("Enter new quantity of supply: ");
        scanf("%d", &suppToUpdate->qty_of_supply);
        printf("Enter new contact number: ");
        scanf("%lld", &suppToUpdate->contact);

        // Update quantity and contact in all linked batches
        for (int i = 0; i < suppToUpdate->med_count; i++)
        {
            if (suppToUpdate->medications[i] != NULL)
            {
                data *med = suppToUpdate->medications[i];
                for (int j = 0; j < med->batch_count; j++)
                {
                    for (int k = 0; k < med->Batch[j].supplier_count; k++)
                    {
                        if (med->Batch[j].suppliers[k] != NULL &&
                            med->Batch[j].suppliers[k]->supplier_id == supplier_id)
                        {
                            med->Batch[j].suppliers[k]->qty_of_supply = suppToUpdate->qty_of_supply;
                            med->Batch[j].suppliers[k]->contact = suppToUpdate->contact; // <- FIXED
                        }
                    }
                }
            }
        }
        printf("Supplier details updated successfully.\n");
        break;

    default:
        printf("Invalid choice.\n");
        break;
    }
}
void searchsupplierByID()
{
    printf("Enter Supplier ID to search: ");
    int supplier_id;
    scanf("%d", &supplier_id);
    supplier *foundSupplier = search_supplier(supplierTree, supplier_id);
    if (foundSupplier != NULL)
    {
        printf("Supplier found:\n");
        printf("ID: %d\n", foundSupplier->supplier_id);
        printf("Name: %s\n", foundSupplier->supp_name);
        printf("Quantity of Supply: %d\n", foundSupplier->qty_of_supply);
        printf("Contact: %lld\n", foundSupplier->contact);
        printf("Medications Supplied:\n");
        for (int i = 0; i < foundSupplier->med_count; i++)
        {
            if (foundSupplier->medications[i] != NULL)
            {
                printf("  - %s (ID: %d)\n",
                       foundSupplier->medications[i]->medname,
                       foundSupplier->medications[i]->medID);
            }
        }
    }
    else
    {
        printf("Supplier with ID %d not found.\n", supplier_id);
    }
}
void supplierManagement()
{
    int choice;
    printf("Enter 1 for adding supplier\nEnter 2 for deleting supplier\nEnter 3 for updating supplier\nEnter 4 for searching supplier\n");
    scanf("%d", &choice);
    switch (choice)
    {
    case 1:
        addsupplier();
        break;
    case 2:
        deleteSupplier(&supplierTree);
        break;
    case 3:
        updateSupplier(supplierTree);
        break;
    case 4:
        searchsupplierByID();
        break;
    }
}

void stockAlerts(B *root)
{
    if (root == NULL)
        return;

    for (int i = 0; i < root->num_keys; i++)
    {
        data *med = root->values[i];
        int totalQtyInStock = 0;

        for (int j = 0; j < med->batch_count; j++)
        {
            totalQtyInStock += med->Batch[j].qty_instock;
        }

        if (totalQtyInStock <= med->reorder_lvl)
        {
            printf("Stock alert: Medication \"%s\" (ID: %d)\n", med->medname, med->medID);
            printf("Current stock: %d | Reorder level: %d\n", totalQtyInStock, med->reorder_lvl);
        }

        stockAlerts(root->children[i]); // Traverse left children
    }

    // Traverse the rightmost child (not handled in loop above)
    stockAlerts(root->children[root->num_keys]);
}
void sortMedicationByExpiry(B_str *node, const char *date1, const char *date2)
{
    if (node == NULL)
        return;

    // Convert input range from DD-MM-YYYY to YYYY-MM-DD
    char start_date[11], end_date[11];
    convert_date_format(date1, start_date);
    convert_date_format(date2, end_date);

    int i = 0;
    while (i < node->num_keys)
    {
        // Traverse left subtree if needed
        if (!node->is_leaf && (i == 0 || strcmp(node->keys[i], start_date) >= 0))
        {
            sortMedicationByExpiry(node->children[i], date1, date2); // reuse original input
        }

        // Check if key lies in [start_date, end_date]
        if (strcmp(node->keys[i], start_date) >= 0 && strcmp(node->keys[i], end_date) <= 0)
        {
            data *med = node->values[i];
            if (med != NULL)
            {
                char formatted_exp[11];
                reverse_date_format(node->keys[i], formatted_exp);

                printf("Medication ID: %d, Name: %s, Expiry Date: %s\n",
                       med->medID, med->medname, formatted_exp);
            }
        }

        i++;
    }

    if (!node->is_leaf)
    {
        if (strcmp(node->keys[node->num_keys - 1], end_date) <= 0)
        {
            sortMedicationByExpiry(node->children[i], date1, date2);
        }
    }
}

void salesTracking(B *node, int medID, int qtySold)
{
    if (node == NULL || qtySold <= 0)
    {
        printf("Invalid request.\n");
        return;
    }

    int i = 0;
    while (i < node->num_keys && medID > node->keys[i])
    {
        i++;
    }

    if (i < node->num_keys && node->keys[i] == medID)
    {
        data *med = node->values[i];
        if (med == NULL || med->batch_count == 0)
        {
            printf("No batches available for medication ID %d.\n", medID);
            return;
        }

        // sort batches so that medications that expire earlier are sold first
        sortBatchesByExpiry(med->Batch, med->batch_count);

        int remaining = qtySold;
        int batches_used = 0;

        for (int j = 0; j < med->batch_count && remaining > 0; j++)
        {
            if (med->Batch[j].qty_instock > 0)
            {
                int sell = (med->Batch[j].qty_instock >= remaining) ? remaining : med->Batch[j].qty_instock;

                med->Batch[j].qty_instock -= sell;
                remaining -= sell;
                batches_used++;

                printf("Sold %d units from batch %d (remaining: %d)\n",
                       sell, med->Batch[j].batch_no, med->Batch[j].qty_instock);

                if (med->Batch[j].qty_instock == 0)
                {
                    med->batch_count--;
                    for (int k = j; k < med->batch_count; k++)
                    {
                        med->Batch[k] = med->Batch[k + 1];
                    }
                    j--; // stay on the new batch at this index
                }
            }
        }

        if (remaining == 0)
        {
            printf("Successfully sold %d units of medication ID %d across %d batches.\n",
                   qtySold, medID, batches_used);
        }
        else
        {
            printf("Partial sale: Only %d/%d units sold for medication ID %d.\n",
                   (qtySold - remaining), qtySold, medID);
        }
        return;
    }

    if (!node->is_leaf)
    {
        salesTracking(node->children[i], medID, qtySold);
    }
    else
    {
        printf("Medication ID %d not found.\n", medID);
    }
}

void collectSupplierTurnovers(B *medRoot, int *turnover, int *supplierIDs)
{
    if (medRoot == NULL)
        return;

    // Traverse left children
    for (int i = 0; i < medRoot->num_keys; i++)
    {
        if (!medRoot->is_leaf)
            collectSupplierTurnovers(medRoot->children[i], turnover, supplierIDs);

        data *med = medRoot->values[i];
        int price = med->priceperunit;

        for (int b = 0; b < med->batch_count; b++)
        {
            batch *bt = &med->Batch[b];

            for (int s = 0; s < bt->supplier_count; s++)
            {
                supplier *supp = bt->suppliers[s];
                int suppID = supp->supplier_id;
                supplierIDs[suppID] = suppID;

                // Now find quantity supplied by this supplier for this batch
                // We'll extract it from the batch entry directly
                // We'll assume the qty supplied is stored in the qty_of_supply field
                // of the supplier struct ONLY for this batch entry (as parsed from file)
                int qty_supplied = supp->qty_of_supply;

                turnover[suppID] += price * qty_supplied;
            }
        }
    }

    // Traverse the rightmost child
    if (!medRoot->is_leaf)
        collectSupplierTurnovers(medRoot->children[medRoot->num_keys], turnover, supplierIDs);
}

void top10largestturnover()
{
    if (!medIDTree) // Assuming this is the root of your B-tree for meds
    {
        printf("Medication tree is empty.\n");
        return;
    }

    int turnover[500] = {0};
    int supplierIDs[500] = {0};

    collectSupplierTurnovers(medIDTree, turnover, supplierIDs);

    // Sort both arrays in ascending order using Heapsort
    Heapsort(turnover, supplierIDs, 500);

    printf("Top 10 suppliers by turnover:\n");
    int found = 0;
    for (int i = 499; i >= 490; i--) // top 10 from the end
    {
        if (turnover[i] > 0)
        {
            printf("Supplier ID: %d, Turnover: %d\n", supplierIDs[i], turnover[i]);
            found++;
        }
    }

    if (found == 0)
    {
        printf("No suppliers have non-zero turnover.\n");
    }
}

void searchMedication()
{
    printf("Enter 1 to search by medID\nEnter 2 to search by medName\nEnter 3 to search by supplier\n");
    int choice;
    scanf("%d", &choice);
    switch (choice)
    {
    case 1:
    {
        printf("Enter medication ID to search: ");
        int medID;
        scanf("%d", &medID);
        searchMedicationBymedId(medID, medIDTree);
        break;
    }
    case 2:
    {
        printf("Enter medication name to search: ");
        char medName[MAX_NAME_LENGTH];
        scanf("%s", medName);
        searchMedicationBymedname(medName, mednameTree);
        break;
    }
    case 3:
    {
        searchbysupplier();
        break;
    }
    }
}
void writeMedicationsToFileRecursive(B *node, FILE *fp)
{
    if (!node || !fp)
        return;

    for (int i = 0; i < node->num_keys; i++)
    {
        if (node->children[i])
            writeMedicationsToFileRecursive(node->children[i], fp);

        data *med = node->values[i];
        if (!med)
            continue;

        // Write medication line
        fprintf(fp, "%d,%s,%d,%d\n", med->medID, med->medname, med->priceperunit, med->reorder_lvl);

        for (int j = 0; j < med->batch_count; j++)
        {
            batch *b = &med->Batch[j];

            // Convert expiry date format before writing
            char reversed_date[MAX_DATE_LENGTH];
            reverse_date_format(b->exp_date, reversed_date);

            fprintf(fp, "%d,%s,%d\n", b->batch_no, reversed_date, b->qty_instock);

            for (int k = 0; k < b->supplier_count; k++)
            {
                supplier *s = b->suppliers[k];
                fprintf(fp, "%d,%s,%d,%lld\n", s->supplier_id, s->supp_name, s->qty_of_supply, s->contact);
            }

            fprintf(fp, "###\n"); // Separate batches
        }

        fprintf(fp, "END\n"); // End of medication
    }

    if (node->children[node->num_keys])
        writeMedicationsToFileRecursive(node->children[node->num_keys], fp);
}

void saveMedicationsToFile()
{
    FILE *fp = fopen("medications_data.txt", "w");
    if (!fp)
    {
        printf("Error opening file for writing.\n");
        return;
    }

    writeMedicationsToFileRecursive(medIDTree, fp);
    fclose(fp);
    printf("Medication data successfully saved to file.\n");
}
void loadMedicationsFromFile()
{
    FILE *fp = fopen("medications_data.txt", "r");
    if (!fp)
    {
        printf("No previous medication data found.\n");
        return;
    }

    char line[256];
    data *currentMed = NULL;
    batch *currentBatch = NULL;

    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "END", 3) == 0)
        {
            currentMed = NULL;
            currentBatch = NULL;
            continue;
        }

        if (strncmp(line, "###", 3) == 0)
        {
            currentBatch = NULL;
            continue;
        }

        if (currentMed == NULL)
        {
            // Medication line
            int medID, priceperunit, reorder_lvl;
            char medname[MAX_NAME_LENGTH];
            sscanf(line, "%d,%[^,],%d,%d", &medID, medname, &priceperunit, &reorder_lvl);

            currentMed = (data *)malloc(sizeof(data));
            currentMed->medID = medID;
            strncpy(currentMed->medname, medname, MAX_NAME_LENGTH);
            currentMed->priceperunit = priceperunit;
            currentMed->reorder_lvl = reorder_lvl;
            currentMed->batch_count = 0;

            insert_int(&medIDTree, medID, currentMed);
            insert_str(&mednameTree, medname, currentMed);
        }
        else if (currentBatch == NULL)
        {
            // Batch line
            int batch_no, qty_instock;
            char exp_date[MAX_DATE_LENGTH];
            sscanf(line, "%d,%[^,],%d", &batch_no, exp_date, &qty_instock);

            if (currentMed->batch_count >= MAX_BATCHES)
                continue;

            currentBatch = &currentMed->Batch[currentMed->batch_count++];
            currentBatch->batch_no = batch_no;

            // Convert date format before storing and indexing
            char formatted_date[MAX_DATE_LENGTH];
            convert_date_format(exp_date, formatted_date);
            strncpy(currentBatch->exp_date, formatted_date, MAX_DATE_LENGTH);

            currentBatch->qty_instock = qty_instock;
            currentBatch->supplier_count = 0;

            insert_str(&expDateTree, currentBatch->exp_date, currentMed);
        }
        else
        {
            // Supplier line
            int supplier_id, qty_supplied;
            long long contact;
            char supp_name[MAX_NAME_LENGTH];
            sscanf(line, "%d,%[^,],%d,%lld", &supplier_id, supp_name, &qty_supplied, &contact);

            supplier *s = search_supplier(supplierTree, supplier_id);

            if (!s)
            {
                s = (supplier *)malloc(sizeof(supplier));
                s->supplier_id = supplier_id;
                strncpy(s->supp_name, supp_name, MAX_NAME_LENGTH);
                s->qty_of_supply = 0;
                s->contact = contact;
                s->med_count = 0;

                suppliers_active[supplier_id] = 1;
                // DON'T insert into B-tree yet — wait till medication is fully linked
            }
            s->qty_of_supply += qty_supplied;

            // Link supplier to current batch
            if (currentBatch->supplier_count < MAX_SUPPLIERS)
                currentBatch->suppliers[currentBatch->supplier_count++] = s;

            // Link medication to supplier if not already added
            Boolean medExists = false;
            for (int i = 0; i < s->med_count; i++)
            {
                if (s->medications[i] && s->medications[i]->medID == currentMed->medID)
                {
                    medExists = true;
                    break;
                }
            }

            if (!medExists && s->med_count < MAX_MEDS_PER_SUPP)
            {
                s->medications[s->med_count++] = currentMed;
            }

            // Now insert supplier into B-tree after all linking
            if (!search_supplier(supplierTree, supplier_id))
            {
                insert_supp(&supplierTree, supplier_id, s); // insert only now
            }
        }
    }

    fclose(fp);
    printf("Medication data successfully loaded from file.\n");
}

int main()
{
    // Load existing data from file
    loadMedicationsFromFile();

    int choice;
    do
    {
        printf("\n=== Pharmacy Management System ===\n");
        printf("1. Add New Medication\n");
        printf("2. Update Medication\n");
        printf("3. Delete Medication\n");
        printf("4. Search Medication\n");
        printf("5. View Stock Alerts\n");
        printf("6. Check Expiry Dates\n");
        printf("7. Sort Medication by Expiry Dates\n");
        printf("8. Track the sales\n");
        printf("9. Supplier Management\n");
        printf("10. Top 10 All-rounder Supplier\n");
        printf("11. Top 10 Largest Turnover\n");
        printf("12. To Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
        {
            addMedicationGeneral(&medIDTree, &mednameTree, &expDateTree, &supplierTree);
            saveMedicationsToFile(medIDTree);
            break;
        }
        case 2:
        {
            int medID;
            printf("Enter Medication ID to update: ");
            scanf("%d", &medID);
            updateMedication(medID);
            saveMedicationsToFile(medIDTree);
            break;
        }
        case 3:
        {
            int medID, batch_no;
            printf("Enter Medication ID to delete: ");
            scanf("%d", &medID);
            printf("Enter Batch No.: ");
            scanf("%d", &batch_no);
            deleteMedication(&medIDTree, &mednameTree, &expDateTree, &supplierTree, medID, batch_no);
            saveMedicationsToFile(medIDTree);
            break;
        }
        case 4:
        {
            searchMedication();
            break;
        }
        case 5:
        {
            stockAlerts(medIDTree);
            break;
        }
        case 6:
        {
            char date[MAX_DATE_LENGTH];
            printf("Enter current date (DD-MM-YYYY): ");
            scanf(" %10s", date);
            checkexpirydate(expDateTree, date);
            break;
        }
        case 7:
        {
            char startDate[11], endDate[11];
            printf("Enter the start date (DD-MM-YYYY): ");
            scanf("%10s", startDate);
            printf("Enter the end date (DD-MM-YYYY): ");
            scanf("%10s", endDate);

            printf("Medications with expiry dates between %s and %s:\n", startDate, endDate);
            sortMedicationByExpiry(expDateTree, startDate, endDate);
            printf("\n");
            break;
        }
        case 8:
        {
            int medID, qtySold;
            printf("Enter Medication ID to sell: ");
            scanf("%d", &medID);
            printf("Enter quantity to sell: ");
            scanf("%d", &qtySold);
            salesTracking(medIDTree, medID, qtySold);
            saveMedicationsToFile(medIDTree);
            break;
        }
        case 9:
        {
            supplierManagement();
            saveMedicationsToFile(medIDTree);
            break;
        }
        case 10:
        {
            top10Allrounders(supplierTree);
            break;
        }
        case 11:
        {
            top10largestturnover(supplierTree);
            break;
        }
        case 12:
            printf("Exiting...\n");
            saveMedicationsToFile(medIDTree);
            break;
        default:
            printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 12);

    return 0;
}