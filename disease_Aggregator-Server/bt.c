#include "bt.h"
#include "func.h"

static int countCurrentPatients = 0;

BTTree *BTCreate() {
    BTTree *tree = (BTTree *) malloc(sizeof (BTTree));
    tree->nodes = 0;
    tree->ptr = NULL;
    return tree;
}

int BTHeight(BTTree * tree) {
    return maxDepth(tree->ptr);
}

int maxDepth(BTNode node) {
    if (node == NULL)
        return 0;
    else {
        /* compute the depth of each subtree */
        int lDepth = maxDepth(node->left);
        int rDepth = maxDepth(node->right);
        /* use the larger one */
        if (lDepth > rDepth)
            return lDepth + 1;
        else return rDepth + 1;
    }
}

int BTSize(BTTree* tree) {
    if (tree == NULL)
        return 0;
    else
        return tree->nodes;
}

BTNode BTNodeUpdate(BTNode node, patientRecord* record) {
    node = malloc(sizeof (BTNodeStruct));
    node->left = NULL;
    node->right = NULL;
    node->pRecord = record;
    strcpy(node->Date, record->entryDate);
    return node;
}

BTNode BTInsert(BTTree *tree, BTNode node, patientRecord *record) {
    if (node == NULL) {
        node = BTNodeUpdate(node, record);
        //PrintPatient(*record); //print patient records to see if it's all ok
        //printf("Inserting date : {%s} at node \n", node->Date);
        tree->nodes++;
        return node;
    }
    if (DateCmp(node->Date, record->entryDate) == 0) {
        /*record entrDate >node->Date -> insert right*/
        node->right = BTInsert(tree, node->right, record);
    } else if (DateCmp(node->Date, record->entryDate) == -1) {
        /*record entrDate < node->Date -> insert left*/
        node->left = BTInsert(tree, node->left, record);
    }

    return node;
}

void DestroyNode(BTTree * tree, BTNode node) {
    if (node == NULL) return;

    DestroyNode(tree, node->left);
    DestroyNode(tree, node->right);

    free(node);
}

void BTDestroy(BTTree *tree) {
    if (tree != NULL) {
        DestroyNode(tree, tree->ptr);
    }
}

char* BTGetDate(BTTree * tree, BTNode node) {
    if (node != NULL) {
        return node->Date;
    } else {
        return NULL;
    }
}

void BTLevelOrder(BTTree *tree) {
    int levels = BTHeight(tree);
    int i;
    for (i = 1; i <= levels; i++)
        printGivenLevel(tree, tree->ptr, i);
}

void printGivenLevel(BTTree* tree, BTNode node, int level) {
    if (node == NULL)
        return;
    if (level == 1)
        printNode(tree, node);
    else if (level > 1) {
        printGivenLevel(tree, node->left, level - 1);
        printGivenLevel(tree, node->right, level - 1);
    }
}

void printNode(BTTree * tree, BTNode node) {
    char* Date = BTGetDate(tree, node);
    printf("%s   ", Date);
}

void BTSearchDatesKey(BTNode node, char* date1, char*date2, int*count, char* key) {
    if (DateCmp(node->Date, date1) == -1) {
        /*dateNode > date1*/
        if (DateCmp(node->Date, date2) == 0) {
            /*dateNode < date2*/
            if (strcmp(key, "-") != 0) {
                if (strcmp(key, node->pRecord->country) == 0)
                    (*count)++;
            } else
                (*count)++;
        }
        if (node->left != NULL) {
            BTSearchDatesKey(node->left, date1, date2, count, key);
        }
        if (node->right != NULL) {
            BTSearchDatesKey(node->right, date1, date2, count, key);
        }
    } else {
        /*dateNode < date1*/
        if (node->right != NULL) {
            BTSearchDatesKey(node->right, date1, date2, count, key);
        }
    }
}

void BTSearchDatesAge(BTNode node, char* date1, char*date2, int*count, int *total_count, char* key) {
    int age = 0;
    if (DateCmp(node->Date, date1) == -1) {
        /*dateNode > date1*/
        if (DateCmp(node->Date, date2) == 0) {
            /*dateNode < date2*/
            if (strcmp(key, node->pRecord->country) == 0) {
                age = atoi(node->pRecord->age);
                (*total_count)++;
                if (age <= 20)
                    (count[0])++;
                else if (age <= 40)
                    (count[1])++;
                else if (age <= 60)
                    (count[2])++;
                else
                    (count[3])++;
            }

        }
        if (node->left != NULL) {
            BTSearchDatesAge(node->left, date1, date2, count, total_count, key);
        }
        if (node->right != NULL) {
            BTSearchDatesAge(node->right, date1, date2, count, total_count, key);
        }
    } else {
        /*dateNode < date1*/
        if (node->right != NULL) {
            BTSearchDatesAge(node->right, date1, date2, count, total_count, key);
        }
    }
}

void BTSearchDatesExit(BTNode node, char* date1, char*date2, int*count, char* key) {
    if (DateCmp(node->Date, date1) == -1) {
        /*dateNode > date1*/
        if (DateCmp(node->Date, date2) == 0) {
            /*dateNode < date2*/
            if (strcmp(key, node->pRecord->country) == 0) {
                if (strcmp(node->pRecord->state, "ENTER/EXIT") == 0 && DateCmp(node->pRecord->exitDate, date2) == 0) {
                    (*count)++;
                }
            }
        }
        if (node->left != NULL) {
            BTSearchDatesExit(node->left, date1, date2, count, key);
        }
        if (node->right != NULL) {
            BTSearchDatesExit(node->right, date1, date2, count, key);
        }
    } else {
        /*dateNode < date1*/
        if (node->right != NULL) {
            BTSearchDatesExit(node->right, date1, date2, count, key);
        }
    }
}

int BTKeyCount(BTTree *tree, char* key, int keyFlag) {
    countCurrentPatients = 0;
    int levels = BTHeight(tree);
    int i;
    for (i = 1; i <= levels; i++)
        KeyCountGivenLevel(tree, tree->ptr, i, key, keyFlag);
    return countCurrentPatients;
}

void KeyCountGivenLevel(BTTree* tree, BTNode node, int level, char* key, int keyFlag) {
    if (node == NULL)
        return;
    if (level == 1)
        KeyCountNode(tree, node, key, keyFlag);
    else if (level > 1) {
        KeyCountGivenLevel(tree, node->left, level - 1, key, keyFlag);
        KeyCountGivenLevel(tree, node->right, level - 1, key, keyFlag);
    }
}

void KeyCountNode(BTTree * tree, BTNode node, char*key, int keyFlag) {
    /*key can be a disease or country*/
    /*keyFlag = 0 if key is disease and 1 if its a country*/
    if (keyFlag == 0) {
        if (strcmp(node->pRecord->diseaseID, key) == 0) {
            countCurrentPatients++;
        }
    } else if (keyFlag == 1) {
        if (strcmp(node->pRecord->country, key) == 0) {
            countCurrentPatients++;
        }
    }
}
