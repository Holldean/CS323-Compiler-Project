struct ExpRecord **varRecords;
struct ExpRecord **tempRecords;
struct ExpRecordNode *recordList;
int varNum;
int tempNum;

enum ExpressionType
{
    _NOTHING = 0, _ONE_CON, _ONE_VAR, _MINUS_ONE_CON, _MINUS_ONE_VAR, _CON_ADD_VAR, _CON_SUB_VAR, _CON_MUL_VAR, _CON_DIV_VAR, 
    _VAR_ADD_CON, _VAR_SUB_CON, _VAR_MUL_CON, _VAR_DIV_CON, _VAR_ADD_VAR, _VAR_SUB_VAR, _VAR_MUL_VAR, _VAR_DIV_VAR
};

typedef struct ExpRecord {
    int left;
    int right;
    enum ExpressionType type;
} ExpRecord;

typedef struct ExpRecordNode {
    int result;
    int left;
    int right;
    enum ExpressionType type;
    struct ExpRecordNode *next;
    struct ExpRecordNode *previous;
} ExpRecordNode;

ExpRecord *new_exp_record() {
    ExpRecord *newRecord = (ExpRecord *)malloc(sizeof(ExpRecord));
    newRecord->left = 0;
    newRecord->right = 0;
    newRecord->type = _NOTHING;
    return newRecord;
}

ExpRecordNode *new_exp_record_node() {
    ExpRecordNode *node = (ExpRecordNode *)malloc(sizeof(ExpRecordNode));
    node->result = 0;
    node->left = 0;
    node->right = 0;
    node->type = _NOTHING;
    node->next = NULL;
    node->previous = NULL;
    return node;
}

void append_exp_to_list(ExpRecordNode *node) {
    if (recordList) {
        recordList->previous = node;
        node->next = recordList;
        recordList = node;
    } else {
        recordList = node;
    }
}

ExpRecordNode *find_available_exp(ExpRecord *record) {
    if (recordList == NULL) {
        return NULL;
    }
    ExpRecordNode *node = recordList;
    while(node) {
        if (record->left == node->left && record->right == node->right && record->type == node->type) {
            return node;
        } else if (record->left == node->right && record->right == node->left) {
            enum ExpressionType typeOne = record->type;
            enum ExpressionType typeTwo = node->type;
            if ((typeOne == _CON_ADD_VAR && typeTwo == _VAR_ADD_CON) 
             || (typeTwo == _CON_ADD_VAR && typeOne == _VAR_ADD_CON)) {
                return node;
            } else if ((typeOne == _CON_MUL_VAR && typeTwo == _VAR_MUL_CON) 
                    || (typeTwo == _CON_MUL_VAR && typeOne == _VAR_MUL_CON)) {
                return node;
            }
        }
        node = node->next;
    }
    return NULL;
}

void remove_exp(int num) {
    if (recordList == NULL) {
        return;
    }
    ExpRecordNode *node = recordList;
    while(node) {
        int jug = 0;
        switch(node->type) {
            case _ONE_VAR:
            case _MINUS_ONE_VAR:
            case _VAR_ADD_CON:
            case _VAR_SUB_CON:
            case _VAR_DIV_CON:
            case _VAR_MUL_CON:
                if (num == node->left)
                    jug = 1;
                break;
            case _CON_DIV_VAR:
            case _CON_MUL_VAR:
            case _CON_SUB_VAR:
            case _CON_ADD_VAR:
                if (num == node->right)
                    jug = 1;
                break;
            case _VAR_ADD_VAR:
            case _VAR_SUB_VAR:
            case _VAR_MUL_VAR:
            case _VAR_DIV_VAR:
                if (num == node->left || num == node->right)
                    jug = 1;
                break;
        }
        if (jug == 1 || num == node->result) {
            if (node->previous) {
                node->previous->next = node->next;
                if (node->next)
                    node->next->previous = node->previous;
            } else {
                recordList = node->next;
                if (node->next)
                    node->next->previous = NULL;
            }
            free(node);
            break;
        }
        node = node->next;
    }
}

void new_block_container() {
    varNum = varNumArrayList->memberNum;
    tempNum = tempNumArrayList->memberNum;
    varRecords = (ExpRecord **)malloc(sizeof(ExpRecord *) * varNum);
    tempRecords = (ExpRecord **)malloc(sizeof(ExpRecord *) * tempNum);
    memset(varRecords, 0, varNum * sizeof(void *));
    memset(tempRecords, 0, tempNum * sizeof(void *));
}

void free_exp_record(ExpRecord *record) {
    record->left = 0;
    record->right = 0;
    record->type = 0;
    free(record);
}

void clear_block_container() {
    int i;
    for (i = 0; i < varNum; i++) {
        if (varRecords[i] != NULL) {
            free_exp_record(varRecords[i]);
        }
    }
    for (i = 0; i < tempNum; i++) {
        if (tempRecords[i] != NULL) {
            free_exp_record(tempRecords[i]);
        }
    }
    ExpRecordNode *node = recordList;
    recordList = NULL;
    while (node) {
        ExpRecordNode *temp = node;
        node = node->next;
        temp->next = NULL;
        temp->previous = NULL;
        free(temp);
    }
}

void free_block_container() {
    varNum = 0;
    tempNum = 0;
    free(varRecords);
    free(tempRecords);
}

ExpRecord *put_record(int num, int left, enum ArgType leftType, int right, enum ArgType rightType, enum InstructType type) {
    ExpRecord *record = new_exp_record();
    if (type == _ASSIGN) {
        if (leftType == _CONSTANT) {
            record->type = _ONE_CON;
        } else {
            record->type = _ONE_VAR;
        }
        record->left = left;
    } else if (type == _MINUS) {
        if (leftType == _CONSTANT) {
            record->type = _MINUS_ONE_CON;
        } else {
            record->type = _MINUS_ONE_VAR;
        }
        record->left = left;
    } else if (leftType == _CONSTANT && rightType == _CONSTANT) {
        int value;
        switch(type) {
            case _PLUS:
                value = left + right;
                break;
            case _MULTIPLY:
                value = left * right;
                break;
            case _SUBSTRACT:
                value = left - right;
                break;
            case _DIVIDE:
                value = left / right;
                break;
        }
        record->type = _ASSIGN;
        record->left = value;
    } else if (leftType == _CONSTANT) {
        switch(type) {
            case _PLUS:
                record->type = _CON_ADD_VAR;
                break;
            case _MULTIPLY:
                record->type = _CON_MUL_VAR;
                break;
            case _SUBSTRACT:
                record->type = _CON_SUB_VAR;
                break;
            case _DIVIDE:
                record->type = _CON_DIV_VAR;
                break;
        }
        record->left = left;
        record->right = right;
    } else if (rightType == _CONSTANT) {
        switch(type) {
            case _PLUS:
                record->type = _VAR_ADD_CON;
                break;
            case _MULTIPLY:
                record->type = _VAR_MUL_CON;
                break;
            case _SUBSTRACT:
                record->type = _VAR_SUB_CON;
                break;
            case _DIVIDE:
                record->type = _VAR_DIV_CON;
                break;
        }
        record->left = left;
        record->right = right;
    } else {
        switch(type) {
            case _PLUS:
                record->type = _VAR_ADD_VAR;
                break;
            case _MULTIPLY:
                record->type = _VAR_MUL_VAR;
                break;
            case _SUBSTRACT:
                record->type = _VAR_SUB_VAR;
                break;
            case _DIVIDE:
                record->type = _VAR_DIV_VAR;
                break;
        }
        record->left = left;
        record->right = right;
    }
    if (num > 0) {
        ExpRecord *temp = varRecords[num - 1];
        if (temp)
            free(temp);
        varRecords[num - 1] = record;
    } else {
        ExpRecord *temp = tempRecords[-num - 1];
        if (temp)
            free(temp);
        tempRecords[-num - 1] = record;
    }
    return record;
}

ExpRecord *get_exp_record(int num) {
    if (num > 0)
        return varRecords[num - 1];
    else
        return tempRecords[-num - 1];
}

ExpRecord *update_record(int num) {
    ExpRecord *record = get_exp_record(num);
    if (record == NULL)
        return NULL;
    switch(record->type) {
        case _ONE_VAR:
            {
                ExpRecord *newRecord = get_exp_record(record->left);
                if (newRecord) {
                    record->left = newRecord->left;
                    record->right = newRecord->right;
                    record->type = newRecord->type;
                }
                return record;
            }
        case _MINUS_ONE_VAR:
            {
                ExpRecord *newRecord = get_exp_record(record->left);
                if (newRecord) {
                    if (newRecord->type == _ONE_VAR) {
                        record->left = newRecord->left;
                        record->type = _MINUS_ONE_VAR;
                    } else if (newRecord->type == _ONE_CON) {
                        record->left = newRecord->left;
                        record->type = _MINUS_ONE_CON;
                    } else if (newRecord->type == _MINUS_ONE_CON) {
                        record->left = newRecord->left;
                        record->type = _ONE_CON;
                    } else if (newRecord->type == _MINUS_ONE_VAR) {
                        record->left = newRecord->left;
                        record->type = _ONE_VAR;
                    } else if (newRecord->type == _VAR_SUB_CON) {
                        record->left = newRecord->left;
                        record->right = newRecord->right;
                        record->type = _CON_SUB_VAR;
                    } else if (newRecord->type == _CON_SUB_VAR) {
                        record->left = newRecord->left;
                        record->right = newRecord->right;
                        record->type = _VAR_SUB_CON;
                    }
                }
                return record;
            }
        case _CON_ADD_VAR:
            {
                int temp;
                temp = record->left;
                record->left = record->right;
                record->right = temp;
                record->type = _VAR_ADD_CON;
                return update_record(num);
            }
        case _CON_MUL_VAR: 
            {
                int temp;
                temp = record->left;
                record->left = record->right;
                record->right = temp;
                record->type = _VAR_MUL_CON;
                return update_record(num);
            }
        case _CON_SUB_VAR:
            {
                ExpRecord *right = get_exp_record(record->right);
                if (right->type == _ONE_CON) {
                    int cal = record->left - right->left;
                    if (cal >= 0) {
                        record->left = cal;
                        record->type = _ONE_CON;
                    } else {
                        record->left = -cal;
                        record->type = _MINUS_ONE_CON;
                    }
                } else if (right->type == _ONE_VAR) {
                    record->right = right->left;
                    record->type = _CON_SUB_VAR;
                } else if (right->type == _MINUS_ONE_CON) {
                    record->left = record->left + right->left;
                    record->type = _ONE_CON;
                } else if (right->type == _MINUS_ONE_VAR) {
                    record->right = right->left;
                    record->type = _CON_ADD_VAR;
                } else if (right->type == _CON_ADD_VAR) {
                    int cal = record->left - right->left;
                    if (cal > 0) {
                        record->left = cal;
                        record->right = right->right;
                        record->type = _CON_SUB_VAR;
                    }
                } else if (right->type == _CON_SUB_VAR) {
                    record->left = right->right;
                    int cal = record->left - right->left;
                    if (cal > 0) {
                        record->right = cal;
                        record->type = _VAR_ADD_CON;
                    } else if (cal < 0) {
                        record->right = -cal;
                        record->type = _VAR_SUB_CON;
                    } else {
                        record->type = _ONE_VAR;
                    }
                } else if (right->type == _VAR_ADD_CON) {
                    int cal = record->left - right->right;
                    if (cal > 0) {
                        record->left = cal;
                        record->right = right->left;
                        record->type = _CON_SUB_VAR;
                    }
                } else if (right->type == _VAR_SUB_CON) {
                    record->left = record->left + right->right;
                    record->right = right->left;
                    record->type = _CON_SUB_VAR;
                }
                return record;
            }
        case _CON_DIV_VAR:
            {
                ExpRecord *right = get_exp_record(record->right);
                if (right->type == _ONE_CON) {
                    record->left = record->left / right->left;
                    record->type = _ONE_CON;   
                } else if (right->type == _ONE_VAR) {
                    record->right = right->left;
                    record->type = _CON_DIV_VAR;
                } else if (right->type == _MINUS_ONE_VAR) {
                    record->left = record->left / right->left;
                    record->type = _MINUS_ONE_CON;
                }
                return record;
            }
        case _VAR_ADD_CON:
            {
                ExpRecord *left = get_exp_record(record->left);
                if (left) {
                    if (left->type == _ONE_CON) {
                        record->left = left->left + record->right;
                        record->type = _ONE_CON;
                    } else if (left->type == _ONE_VAR) {
                        record->left = left->left;
                        record->type = _VAR_ADD_CON;
                    } else if (left->type == _MINUS_ONE_CON) {
                        record->left = record->right - left->left;
                        record->type = _ONE_CON;
                    } else if (left->type == _MINUS_ONE_VAR) {
                        record->left = record->right;
                        record->right = left->left;
                        record->type = _CON_SUB_VAR;
                    } else if (left->type == _CON_ADD_VAR) {
                        record->right = left->left + record->right;
                        record->left = left->right;
                        record->type = _VAR_ADD_CON;
                    } else if (left->type == _CON_SUB_VAR) {
                        record->left = left->left + record->right;
                        record->right = left->right;
                        record->type = _CON_SUB_VAR;
                    } else if (left->type == _VAR_ADD_CON) {
                        record->left = left->left;
                        record->right = record->right + left->right;
                        record->type = _VAR_ADD_CON;
                    } else if (left->type == _VAR_SUB_CON) {
                        record->left = left->left;
                        int cal = record->right - left->right;
                        if (cal > 0) {
                            record->right = cal;
                            record->type = _VAR_ADD_CON;
                        } else if (cal < 0) {
                            record->right = -cal;
                            record->type = _VAR_SUB_CON;
                        } else {
                            record->right = 0;
                            record->type = _ONE_VAR;
                        }
                    }
                }
                return record;
            }
        case _VAR_MUL_CON:
            {
                ExpRecord *left = get_exp_record(record->left);
                if (left) {
                    if (left->type == _ONE_CON) {
                        record->left = left->left * record->right;
                        record->type = _ONE_CON;
                    } else if (left->type == _ONE_VAR) {
                        record->left = left->left;
                        record->type = _VAR_MUL_CON;
                    } else if (left->type == _MINUS_ONE_CON) {
                        record->left = -left->left * record->right;
                        record->type = _MINUS_ONE_CON;
                    } else if (left->type == _CON_MUL_VAR) {
                        record->left = left->left * record->right;
                        record->right = left->right;
                        record->type = _CON_MUL_VAR;
                    } else if (left->type == _VAR_MUL_CON) {
                        record->left = left->left;
                        record->right = left->right * record->right;  
                        record->type = _VAR_MUL_CON;
                    }
                }
                return record;
            }
        case _VAR_SUB_CON:
            {
                ExpRecord *left = get_exp_record(record->left);
                if (left) {
                    if (left->type == _ONE_CON) {
                        int cal = left->left - record->right;
                        if (cal >= 0) {
                            record->left = cal;
                            record->type = _ONE_CON;
                        } else if (cal < 0) {
                            record->left = -cal;
                            record->type = _MINUS_ONE_CON;
                        }
                    } else if (left->type == _ONE_VAR) {
                        record->left = left->left;
                        record->type = _VAR_SUB_CON;
                    } else if (left->type == _MINUS_ONE_CON) {
                        record->left = left->left + record->left;
                        record->type = _MINUS_ONE_CON;
                    } else if (left->type == _CON_ADD_VAR) {
                        record->left = left->right;
                        int cal = left->left - record->right;
                        if (cal > 0) {
                            record->right = cal;
                            record->type = _VAR_ADD_CON;
                        } else if (cal < 0) {
                            record->right = -cal;
                            record->type = _VAR_SUB_CON;
                        } else {
                            record->right = 0;
                            record->type = _ONE_VAR;
                        }
                    } else if (left->type == _CON_SUB_VAR) {
                        int cal = left->left - record->right;
                        if (cal > 0) {
                            record->left = cal;
                            record->right = left->right;
                            record->type = _CON_SUB_VAR;
                        } else if (cal == 0) {
                            record->left = left->right;
                            record->type = _MINUS_ONE_VAR;
                        }
                    } else if (left->type == _VAR_ADD_CON) {
                        record->left = left->left;
                        int cal = left->right - record->right;
                        if (cal > 0) {
                            record->right = cal;
                            record->type = _VAR_ADD_CON;
                        } else if (cal < 0) {
                            record->right = -cal;
                            record->type = _VAR_SUB_CON;
                        } else {
                            record->right = 0;
                            record->type = _ONE_VAR;
                        }
                    } else if (left->type == _VAR_SUB_CON) {
                        record->left = left->left;
                        record->right = left->right + record->right;
                        record->type = _VAR_SUB_CON;
                    }
                }
                return record;
            }
        case _VAR_DIV_CON:
            {
                ExpRecord *left = get_exp_record(record->left);
                if (left) {
                    if (left->type == _ONE_CON) {
                        record->left = left->left / record->left;
                        record->type = _ONE_CON;
                    } else if (left->type == _ONE_VAR) {
                        record->left = left->left;
                        record->type = _VAR_DIV_CON;
                    } else if (left->type == _MINUS_ONE_CON) {
                        record->left = left->left / record->left;
                        record->type = _MINUS_ONE_CON;
                    }
                }
                return record;
            }
        case _VAR_ADD_VAR:
            {
                ExpRecord *left = get_exp_record(record->left);
                ExpRecord *right = get_exp_record(record->right);
                if (left == NULL && right == NULL) {
                    return record;
                } else if (left == NULL && right) {
                    if (right->type == _ONE_CON) {
                        record->right = right->left;
                        record->type = _VAR_ADD_CON;
                    } else if (right->type == _ONE_VAR) {
                        record->right = right->left;
                        record->type = _VAR_ADD_VAR;
                    } else if (right->type == _CON_SUB_VAR) {
                        if (record->left == right->right) {
                            record->left = right->left;
                            record->type = _ONE_CON;
                        }
                    } else if (right->type == _VAR_SUB_VAR) {
                        if (record->left == right->right) {
                            record->left = right->left;
                            record->type = _ONE_VAR;
                        }
                    }
                    return record;
                } else if (left && right == NULL) {
                    if (left->type == _ONE_CON) {
                        record->left = left->left;
                        record->type = _CON_ADD_VAR;
                    } else if (left->type == _ONE_VAR) {
                        record->left = left->left;
                        record->type = _VAR_ADD_VAR;
                    } else if (left->type == _CON_SUB_VAR) {
                        if (left->right == record->right) {
                            record->left = left->left;
                            record->type = _ONE_CON;
                        }
                    } else if (left->type == _VAR_SUB_VAR) {
                        if (left->right == record->right) {
                            record->left = left->left;
                            record->type = _ONE_VAR;
                        }
                    }
                    return record;
                }
                if (left->type == _ONE_CON) {
                    if (right->type == _ONE_CON) {
                        record->left = left->left + right->left;
                        record->type = _ONE_CON;
                    } else if (right->type == _ONE_VAR) {
                        record->left = right->left;
                        record->right = left->left;
                        record->type = _VAR_ADD_CON;
                    } else if (right->type == _MINUS_ONE_CON) {
                        int cal = left->left - right->left;
                        record->type = _ONE_CON;
                        if (cal >= 0) {
                            record->left = cal;
                        } else {
                            record->left = -cal;
                        }
                    } else if (right->type == _MINUS_ONE_VAR) {
                        record->left = left->left;
                        record->right = right->left;
                        record->type = _CON_SUB_VAR;
                    } else if (right->type == _CON_ADD_VAR) {
                        record->left = left->left + right->left;
                        record->right = right->right;
                        record->type = _CON_ADD_VAR;
                    } else if (right->type == _CON_SUB_VAR) {
                        record->left = left->left + right->left;
                        record->right = right->right;
                        record->type = _CON_SUB_VAR;
                    } else if (right->type == _VAR_ADD_CON) {
                        record->left = left->left + right->right;
                        record->right = right->left;
                        record->type = _CON_ADD_VAR;
                    } else if (right->type == _VAR_SUB_CON) {
                        record->left = right->left;
                        int cal = left->left - right->right;
                        if (cal > 0) {
                            record->right = cal;
                            record->type = _VAR_ADD_CON;
                        } else if (cal < 0) {
                            record->right = -cal;
                            record->type= _VAR_SUB_CON;
                        } else {
                            record->right = 0;
                            record->type = _ONE_VAR;
                        }
                    }
                } else if (left->type == _ONE_VAR) {
                    if (right->type == _ONE_CON) {
                        record->left = right->left;
                        record->right = left->left;
                        record->type = _CON_ADD_VAR;
                    } else if (right->type == _ONE_VAR) {
                        record->left = left->left;
                        record->right = right->left;
                        record->type = _VAR_ADD_VAR;
                    } else if (right->type == _MINUS_ONE_CON) {
                        record->left = left->left;
                        record->right = right->left;
                        record->type = _VAR_SUB_CON;
                    } else if (right->type == _MINUS_ONE_VAR) {
                        record->left = left->left;
                        record->right = right->left;
                        record->type = _VAR_SUB_VAR;
                    } else if (right->type == _CON_SUB_VAR) {
                        if (right->right == left->left) {
                            record->left = right->left;
                            record->type = _ONE_CON;
                        }
                    } else if (right->type == _VAR_SUB_VAR) {
                        if (right->right == left->left) {
                            record->left = right->left;
                            record->type = _ONE_VAR;
                        }
                    }
                } else if (left->type == _MINUS_ONE_CON) {
                    if (right->type == _ONE_CON) {
                        int cal = right->left - left->left;
                        record->type = _ONE_CON;
                        if (cal >= 0) {
                            record->left = cal;
                        } else {
                            record->left = -cal;
                        }
                    } else if (right->type == _ONE_VAR) {
                        record->left = right->left;
                        record->right = left->left;
                        record->type = _VAR_SUB_CON;
                    } else if (right->type == _MINUS_ONE_CON) {
                        record->left = left->left + right->left;
                        record->type = _MINUS_ONE_CON;
                    } else if (right->type == _CON_ADD_VAR) {
                        int cal = right->left - left->left;
                        record->left = right->right;
                        if (cal > 0) {
                            record->right = cal;
                            record->type = _VAR_ADD_CON;
                        } else if (cal < 0) {
                            record->right = -cal;
                            record->type = _VAR_SUB_CON;
                        } else {
                            record->type = _ONE_VAR;
                        }
                    } else if (right->type == _CON_SUB_VAR) {
                        int cal = right->left - left->left;
                        if (cal > 0) {
                            record->left = cal;
                            record->right = right->right;
                            record->type = _CON_SUB_VAR;
                        } else if (cal == 0) {
                            record->left = right->right;
                            record->type = _MINUS_ONE_VAR;
                        } 
                    } else if (right->type == _VAR_ADD_CON) {
                        int cal = right->right - left->left;
                        record->left = right->left;
                        if (cal > 0) {
                            record->right = cal;
                            right->type = _VAR_ADD_CON;
                        } else if (cal < 0) {
                            record->right = -cal;
                            right->type = _VAR_SUB_CON;
                        } else {
                            record->type = _ONE_VAR;
                        }
                    } else if (right->type == _VAR_SUB_CON) {
                        record->left = right->left;
                        record->right = left->left + right->right;
                        record->type = _VAR_SUB_VAR;
                    }
                } else if (left->type == _MINUS_ONE_VAR) {
                    if (right->type == _ONE_CON) {
                        record->left = right->left;
                        record->right = left->left;
                        record->type = _CON_SUB_VAR;
                    } else if (right->type == _ONE_VAR) {
                        record->left = right->left;
                        record->right = left->left;
                        record->type = _VAR_SUB_VAR;
                    } else if (right->type == _VAR_ADD_CON) {
                        if (left->left == right->left) {
                            record->left = right->right;
                            record->type = _ONE_CON;
                        }
                    } else if (right->type == _VAR_SUB_CON) {
                        if (left->left == right->left) {
                            record->left = right->right;
                            record->type = _MINUS_ONE_CON;
                        }
                    } else if (right->type == _VAR_ADD_VAR) {
                        if (left->left == right->left) {
                            record->left = right->right;
                            record->type = _ONE_VAR;
                        } else if (left->left == right->right) {
                            record->left = right->left;
                            record->type = _ONE_VAR;
                        }
                    } else if (right->type == _VAR_SUB_VAR) {
                        if (left->left == right->left) {
                            record->left = right->right;
                            record->type = _MINUS_ONE_CON;
                        }
                    }
                } else if (left->type == _CON_ADD_VAR) {
                    if (right->type == _ONE_CON) {
                        record->left = left->left + right->left;
                        record->type = _CON_ADD_VAR;
                    } else if (right->type == _MINUS_ONE_CON) {
                        int cal = left->left - right->left;
                        record->left = right->left;
                        if (cal > 0) {
                            record->right = cal;
                            record->type = _VAR_ADD_CON;
                        } else if (cal < 0) {
                            record->right = -cal;
                            record->type = _VAR_SUB_CON;
                        } else {
                            record->type = _ONE_VAR;
                        }
                    } else if (right->type == _VAR_SUB_CON) {
                        int cal = left->left - right->right;
                        if (cal == 0) {
                            record->left = left->right;
                            record->right = right->left;
                            record->type = _VAR_ADD_VAR;
                        }
                    } else if (right->type == _VAR_SUB_VAR) {
                        if (right->right == left->right) {
                            record->left = left->left;
                            record->right = right->left;
                            record->type = _VAR_ADD_VAR;
                        }
                    }
                } else if (left->type == _CON_SUB_VAR) {
                    if (right->type == _ONE_CON) {
                        record->left = left->left + right->left;
                        record->right = left->right;
                        record->type = _CON_SUB_VAR;
                    } else if (right->type == _ONE_VAR) {
                        if (left->right == right->left) {
                            record->left = left->left;
                            record->type = _ONE_CON;
                        }
                    } else if (right->type == _MINUS_ONE_CON) {
                        int cal = left->left - right->left;
                        if (cal > 0) {
                            record->left = cal;
                            record->right = left->right;
                            record->type = _CON_SUB_VAR;
                        } else if (cal == 0) {
                            record->left = left->right;
                            record->type = _MINUS_ONE_VAR;
                        }
                    } else if (right->type == _CON_ADD_VAR) {
                        if (left->right == right->right) {
                            record->left = left->left + right->left;
                            record->type = _ONE_CON;
                        }
                    } else if (right->type == _VAR_ADD_CON) {
                        if (left->right == right->left) {
                            record->left = left->left + right->right;
                            record->type = _ONE_CON;
                        }
                    } else if (right->type == _VAR_SUB_CON) {
                        if (left->left == right->right && left->right != right->left) {
                            record->left = right->left;
                            record->right = left->right;
                            record->type = _VAR_SUB_VAR;
                        } else if (left->left == right->right && left->right == right->left) {
                            record->left = 0;
                            record->type = _ONE_CON;
                        } else if (left->left != right->right && left->right == right->left) {
                            int cal = left->left - right->right;
                            if (cal >= 0) {
                                record->left = cal;
                                record->type = _ONE_CON;
                            } else {
                                record->left = -cal;
                                record->type = _MINUS_ONE_CON;
                            }
                        }
                    } else if (right->type == _VAR_ADD_VAR) {
                        if (left->right == right->left) {
                            record->left = right->right;
                            record->right = left->left;
                            record->type = _VAR_ADD_CON;
                        } else if (left->right == right->right) {
                            record->left = right->left;
                            record->right = left->left;
                            record->type = _VAR_ADD_CON;
                        }
                    } else if (right->type == _VAR_SUB_VAR) {
                        if (left->right == right->left) {
                            record->left = left->left;
                            record->right = right->right;
                            record->type = _CON_SUB_VAR;
                        }
                    }
                } else if (left->type == _VAR_ADD_CON) {
                    if (right->type == _ONE_CON) {
                        record->left = left->left;
                        record->right = left->right + right->left;
                        record->type = _VAR_ADD_CON;
                    } else if (right->type == _MINUS_ONE_CON) {
                        int cal = left->right - right->left;
                        record->left = left->left;
                        if (cal > 0) {
                            record->right = cal;
                            record->type = _VAR_ADD_CON;
                        } else if (cal < 0) {
                            record->right = -cal;
                            record->type = _VAR_SUB_CON;
                        }
                    } else if (right->type == _MINUS_ONE_VAR) {
                        if (left->left == right->left) {
                            record->left = left->right;
                            record->type = _ONE_CON;
                        }
                    } else if (right->type == _CON_SUB_VAR) {
                        if (left->left == right->right) {
                            record->left = left->right + right->left;
                            record->type = _ONE_CON;
                        }
                    } else if (right->type == _VAR_SUB_CON) {
                        if (left->right == right->right) {
                            record->left = left->left;
                            record->right = right->left;
                            record->type = _VAR_ADD_VAR;
                        }
                    } else if (right->type == _VAR_SUB_VAR) {
                        if (left->left == right->right) {
                            record->left = right->left;
                            record->right = left->right;
                            record->type = _VAR_ADD_CON;
                        }
                    }
                } else if (left->type == _VAR_SUB_CON) {
                    if (right->type == _ONE_CON) {
                        int cal = right->left - left->right;
                        record->left = left->left;
                        if (cal > 0) {
                            record->right = cal;
                            record->type = _VAR_ADD_CON;
                        } else if (cal < 0) {
                            record->right = -cal;
                            record->type = _VAR_SUB_CON;
                        } else {
                            record->type = _ONE_VAR;
                        }
                    } else if (right->type == _MINUS_ONE_CON) {
                        record->left = left->left;
                        record->right = left->right + right->left;
                        record->type = _VAR_SUB_CON;
                    } else if (right->type == _MINUS_ONE_VAR) {
                        if (left->left == right->left) {
                            record->left = left->right;
                            record->type = _MINUS_ONE_CON;
                        }
                    } else if (right->type == _CON_ADD_VAR) {
                        if (left->right == right->left) {
                            record->left = left->left;
                            record->right = right->right;
                            record->type = _VAR_ADD_VAR;
                        }
                    } else if (right->type == _CON_SUB_VAR) {
                        if (left->right == right->left && left->left != right->right) {
                            record->left = left->left;
                            record->right = right->right;
                            record->type = _VAR_SUB_VAR;
                        } else if (left->right == right->left && left->left == right->right) {
                            record->left = 0;
                            record->type = _ONE_CON;
                        } else if (left->right != right->left && left->left == right->right) {
                            int cal = right->left - left->right;
                            if (cal >= 0) {
                                record->left = cal;
                                record->type = _ONE_CON;
                            } else {
                                record->left = -cal;
                                record->type = _MINUS_ONE_CON;
                            }
                        }
                    } else if (left->type == _VAR_ADD_VAR) {
                        if (right->type == _MINUS_ONE_VAR) {
                            if (left->left == right->left) {
                                record->left = left->right;
                                record->type = _ONE_VAR;
                            } else if (left->right == right->left) {
                                record->left = left->left;
                                record->type = _ONE_VAR;
                            }
                        } else if (right->type == _CON_SUB_VAR) {
                            if (left->left == right->right) {
                                record->left = left->right;
                                record->right = right->left;
                                record->type = _VAR_ADD_CON;
                            } else if (left->right == right->right) {
                                record->left = left->left;
                                record->right = right->left;
                                record->type = _VAR_ADD_CON;
                            }
                        } else if (right->type == _VAR_SUB_VAR) {
                            if (left->left == right->right) {
                                record->left = left->right;
                                record->right = right->left;
                                record->type = _VAR_ADD_VAR;
                            } else if (left->right == right->right) {
                                record->left = left->left;
                                record->right = right->left;
                                record->type = _VAR_ADD_VAR;
                            }
                        }
                    } else if (left->type == _VAR_SUB_VAR) {
                        if (right->type == _ONE_VAR) {
                            if (left->right == right->left) {
                                record->left = left->left;
                                record->type = _ONE_VAR;
                            }
                        } else if (right->type == _MINUS_ONE_VAR) {
                            if (left->left == right->left) {
                                record->left = left->right;
                                record->type = _MINUS_ONE_VAR;
                            }
                        } else if (right->type == _CON_ADD_VAR) {
                            if (left->right == right->right) {
                                record->left = left->left;
                                record->right = right->left;
                                record->type = _VAR_ADD_CON;
                            }
                        } else if (right->type == _CON_SUB_VAR) {
                            if (left->left == right->right) {
                                record->left = right->left;
                                record->right = left->right;
                                record->type = _CON_SUB_VAR;
                            }
                        } else if (right->type == _VAR_ADD_CON) {
                            if (left->right == right->left) {
                                record->left = left->left;
                                record->right = right->right;
                                record->type = _VAR_ADD_CON;
                            }
                        } else if (right->type == _VAR_SUB_CON) {
                            if (left->right == right->left) {
                                record->left = left->left;
                                record->right = right->right;
                                record->type = _VAR_SUB_CON;
                            }
                        } else if (right->type == _VAR_ADD_VAR) {
                            if (left->right == right->left) {
                                record->left = left->left;
                                record->right = right->right;
                                record->type = _VAR_ADD_VAR;
                            } else if (left->right == right->right) {
                                record->left = left->left;
                                record->right = right->left;
                                record->type = _VAR_ADD_VAR;
                            }
                        } else if (right->type == _VAR_SUB_VAR) {
                            if (left->left == right->right && left->right == right->left) {
                                record->left = 0;
                                record->type = _ONE_CON;
                            } else if (left->left == right->right && left->right != right->left) {
                                record->left = right->left;
                                record->right = left->right;
                                record->type = _VAR_SUB_VAR;
                            } else if (left->left != right->right && left->right == right->left) {
                                record->left = left->left;
                                record->right = right->right;
                                record->type = _VAR_SUB_VAR;
                            }
                        }
                    }
                }
                return record;
            }
        case _VAR_MUL_VAR:
            {
                ExpRecord *left = get_exp_record(record->left);
                ExpRecord *right = get_exp_record(record->right);
                if (left == NULL && right == NULL) {
                    return record;
                } else if (left == NULL && right) {
                    if (right->type == _ONE_CON) {
                        record->right = right->left;
                        record->type = _VAR_MUL_CON;
                    } else if (right->type == _ONE_VAR) {
                        record->right = right->left;
                        record->type = _VAR_MUL_VAR;
                    }
                    return record;
                } else if (left && right == NULL) {
                    if (left->type == _ONE_CON) {
                        record->left = left->left;
                        record->type = _CON_MUL_VAR;
                    } else if (left->type == _ONE_VAR) {
                        record->left = left->left;
                        record->type = _VAR_MUL_VAR;
                    }
                    return record;
                }
                if (left->type == _ONE_CON) {
                    if (right->type == _ONE_CON) {
                        record->left = left->left * right->left;
                        record->type = _ONE_CON;
                    } else if (right->type == _ONE_VAR) {
                        record->left = left->left;
                        record->right = right->left;
                        record->type = _CON_MUL_VAR;
                    } else if (right->type == _MINUS_ONE_CON) {
                        record->left = left->left * right->left;
                        record->type = _MINUS_ONE_CON;
                    } else if (right->type == _CON_MUL_VAR) {
                        record->left = left->left * right->left;
                        record->right = right->right;
                        record->type = _CON_MUL_VAR;
                    }
                } else if (left->type == _ONE_VAR) {
                    if (right->type == _ONE_CON) {
                        record->left = left->left;
                        record->right = right->left;
                        record->type = _VAR_MUL_CON;
                    } else if (right->type == _ONE_VAR) {
                        record->left = left->left;
                        record->right = right->left;
                        record->type = _VAR_MUL_VAR;
                    }
                } else if (left->type == _MINUS_ONE_CON) {
                    if (right->type == _ONE_CON) {
                        record->left = left->left * right->left;
                        record->type = _MINUS_ONE_CON;
                    } else if (right->type == _MINUS_ONE_CON) {
                        record->left = left->left * right->left;
                        record->type = _ONE_CON;
                    } else if (right->type == _MINUS_ONE_VAR) {
                        record->left = left->left;
                        record->right = right->right;
                        record->type = _CON_MUL_VAR;
                    }
                } else if (left->type == _MINUS_ONE_VAR) {
                    if (right->type == _MINUS_ONE_CON) {
                        record->left = left->left;
                        record->right = right->left;
                        record->type = _VAR_MUL_CON;
                    } else if (right->type == _MINUS_ONE_VAR) {
                        record->left = left->left;
                        record->right = right->left;
                        record->type = _VAR_MUL_VAR;
                    }
                } else if (left->type == _CON_MUL_VAR) {
                    if (right->type == _ONE_CON) {
                        record->left = left->left * right->left;
                        record->right = left->right;
                        record->type = _CON_MUL_VAR;
                    }
                } else if (left->type == _VAR_MUL_CON) {
                    if (right->type == _ONE_CON) {
                        record->left = left->left;
                        record->right = left->right * right->left;
                        record->type = _VAR_MUL_CON;
                    }
                }
                return record;
            }
        case _VAR_SUB_VAR:
            {
                ExpRecord *left = get_exp_record(record->left);
                ExpRecord *right = get_exp_record(record->right);
                if (left == NULL && right == NULL) {
                    if (record->left == record->right) {
                        record->left = 0;
                        record->type = _ONE_CON;
                    }
                    return record;
                } else if (left == NULL && right) {
                    if (right->type == _ONE_CON) {
                        record->right = right->left;
                        record->type = _VAR_SUB_CON;
                    } else if (right->type == _ONE_VAR) {
                        record->right = right->left;
                        record->type = _VAR_SUB_VAR;
                    } else if (right->type == _MINUS_ONE_CON) {
                        record->right = right->left;
                        record->type = _VAR_ADD_CON;
                    } else if (right->type == _MINUS_ONE_VAR) {
                        record->right = right->left;
                        record->type = _VAR_ADD_VAR;
                    } else if (right->type == _CON_ADD_VAR) {
                        if (record->left == right->right) {
                            record->left = right->left;
                            record->type = _MINUS_ONE_CON;
                        }
                    } else if (right->type == _VAR_ADD_CON) {
                        if (record->left == right->left) {
                            record->left = right->right;
                            record->type = _MINUS_ONE_CON;
                        }
                    } else if (right->type == _VAR_SUB_CON) {
                        if (record->left == right->left) {
                            record->left = right->right;
                            record->type = _ONE_CON;
                        }
                    } else if (right->type == _VAR_ADD_VAR) {
                        if (record->left == right->left) {
                            record->left = right->right;
                            record->type = _MINUS_ONE_VAR;
                        } else if (record->left == right->right) {
                            record->left = right->left;
                            record->type = _MINUS_ONE_VAR;
                        }
                    } else if (right->type == _VAR_SUB_VAR) {
                        if (record->left == right->left) {
                            record->left = right->right;
                            record->type = _ONE_VAR;
                        }
                    }
                    return record;
                } else if (left && right == NULL) {
                    if (left->type == _ONE_CON) {
                        record->left = left->left;
                        record->type = _CON_SUB_VAR;
                    } else if (left->type == _ONE_VAR) {
                        record->left = left->left;
                        record->type = _VAR_SUB_VAR;
                    } else if (left->type == _CON_ADD_VAR) {
                        if (left->right == record->right) {
                            record->left = left->left;
                            record->type = _ONE_CON;
                        }
                    } else if (left->type == _VAR_ADD_CON) {
                        if (left->left == record->right) {
                            record->left = left->right;
                            record->type = _ONE_CON;
                        }
                    } else if (left->type == _VAR_SUB_CON) {
                        if (left->left == record->right) {
                            record->left = right->right;
                            record->type = _MINUS_ONE_CON;
                        }
                    } else if (left->type == _VAR_ADD_VAR) {
                        if (left->left == record->right) {
                            record->left = left->right;
                            record->type = _ONE_VAR;
                        } else if (left->right == record->right) {
                            record->left = left->left;
                            record->type = _ONE_VAR;
                        }
                    } else if (left->type == _VAR_SUB_VAR) {
                        if (left->left == record->right) {
                            record->left = left->right;
                            record->type = _MINUS_ONE_VAR;
                        }
                    }
                    return record;
                }
                if (left->type == _ONE_CON) {
                    if (right->type == _ONE_CON) {
                        int cal = left->left - right->left;
                        if (cal >= 0) {
                            record->left = cal;
                            record->type = _ONE_CON;
                        } else {
                            record->left = -cal;
                            record->type = _MINUS_ONE_CON;
                        }
                    } else if (right->type == _ONE_VAR) {
                        record->left = left->left;
                        record->right = right->left;
                        record->type = _CON_SUB_VAR;
                    } else if (right->type == _MINUS_ONE_CON) {
                        record->left = left->left + right->left;
                        record->type = _ONE_CON;
                    } else if (right->type == _MINUS_ONE_VAR) {
                        record->left = left->left;
                        record->right = right->left;
                        record->type = _CON_ADD_VAR;
                    } else if (right->type == _CON_ADD_VAR) {
                        int cal = left->left - right->left;
                        if (cal > 0) {
                            record->left = cal;
                            record->right = right->right;
                            record->type = _CON_SUB_VAR;
                        }
                    } else if (right->type == _CON_SUB_VAR) {
                        int cal = left->left - right->left;
                        record->left = right->right;
                        if (cal > 0) {
                            record->right = cal;
                            record->type = _VAR_ADD_CON;
                        } else if (cal < 0) {
                            record->right = -cal;
                            record->type = _VAR_SUB_CON;
                        } else {
                            record->type = _ONE_VAR;
                        }
                    } else if (right->type == _VAR_ADD_CON) {
                        int cal = left->left - right->right;
                        if (cal > 0) {
                            record->left = cal;
                            record->right = right->left;
                            record->type = _CON_SUB_VAR;
                        }
                    } else if (right->type == _VAR_SUB_CON) {
                        record->left = left->left + right->right;
                        record->right = right->left;
                        record->type = _CON_SUB_VAR;
                    }
                } else if (left->type == _ONE_VAR) {
                    if (right->type == _ONE_CON) {
                        record->left = left->left;
                        record->right = right->right;
                        record->type = _VAR_SUB_CON;
                    } else if (right->type == _ONE_VAR) {
                        if (left->left == right->left) {
                            record->left = 0;
                            record->type = _ONE_CON;
                        } else {
                            record->left = left->left;
                            record->right = right->left;
                            record->type = _VAR_SUB_VAR;
                        }
                    } else if (right->type == _MINUS_ONE_CON) {
                        record->left = left->left;
                        record->right = right->left;
                        record->type = _VAR_ADD_CON;
                    } else if (right->type == _MINUS_ONE_VAR) {
                        record->left = left->left;
                        record->right = right->left;
                        record->type = _VAR_ADD_VAR;
                    } else if (right->type == _CON_ADD_VAR) {
                        if (left->left == right->right) {
                            record->left = right->left;
                            record->type = _MINUS_ONE_CON;
                        }
                    } else if (right->type == _VAR_ADD_CON) {
                        if (left->left == right->left) {
                            record->left = right->right;
                            record->type = _MINUS_ONE_CON;
                        }
                    } else if (right->type == _VAR_SUB_CON) {
                        if (left->left == right->left) {
                            record->left = right->right;
                            record->type = _ONE_CON;
                        }
                    } else if (right->type == _VAR_ADD_VAR) {
                        if (left->left == right->left) {
                            record->left = right->right;
                            record->type = _MINUS_ONE_VAR;
                        } else if (left->left == right->right) {
                            record->left = right->left;
                            record->type = _MINUS_ONE_VAR;
                        }
                    } else if (right->type == _VAR_SUB_VAR) {
                        if (left->left == right->left) {
                            record->left = right->right;
                            record->type = _ONE_VAR;
                        }
                    }
                } else if (left->type == _MINUS_ONE_CON) {
                    if (right->type == _ONE_CON) {
                        record->left = left->left + right->left;
                        record->type = _MINUS_ONE_CON;
                    } else if (right->type == _MINUS_ONE_CON) {
                        int cal = right->left - left->left;
                        if (cal >= 0) {
                            record->left = cal;
                            record->type = _ONE_CON;
                        } else if (cal < 0) {
                            record->left = -cal;
                            record->type = _MINUS_ONE_CON;
                        }
                    } else if (right->type == _MINUS_ONE_VAR) {
                        record->left = right->left;
                        record->right = left->left;
                        record->type = _VAR_SUB_CON;
                    } else if (right->type == _CON_SUB_VAR) {
                        record->left = right->right;
                        record->right = left->left + right->left;
                        record->type = _VAR_SUB_CON;
                    } else if (right->type == _VAR_SUB_CON) {
                        if (right->right == left->left) {
                            record->left = right->left;
                            record->type = _MINUS_ONE_VAR;
                        }
                    }
                } else if (left->type == _MINUS_ONE_VAR) {
                    if (right->type = _MINUS_ONE_CON) {
                        record->left = right->left;
                        record->right = left->left;
                        record->type = _CON_SUB_VAR;
                    } else if (right->type == _MINUS_ONE_VAR) {
                        record->left = right->left;
                        record->right = left->left;
                        record->type = _VAR_SUB_VAR;
                    } else if (right->type == _CON_SUB_VAR) {
                        if (left->left == right->right) {
                            record->left = right->left;
                            record->type = _MINUS_ONE_CON;
                        }
                    } else if (right->type == _VAR_SUB_VAR) {
                        if (left->left == right->right) {
                            record->left = right->left;
                            record->type = _MINUS_ONE_VAR;
                        }
                    }
                } else if (left->type == _CON_ADD_VAR) {
                    if (right->type == _ONE_CON) {
                        record->left = left->right;
                        int cal = left->left - right->left;
                        if (cal > 0) {
                            record->right = cal;
                            record->type = _VAR_ADD_CON;
                        } else if (cal < 0) {
                            record->right = -cal;
                            record->type = _VAR_SUB_CON;
                        } else {
                            record->type = _ONE_VAR;
                        }
                    } else if (right->type == _ONE_VAR) {
                        if (left->right == right->left) {
                            record->left = left->left;
                            record->type = _ONE_CON;
                        }
                    } else if (right->type == _MINUS_ONE_CON) {
                        record->left = left->left + right->left;
                        record->right = left->right;
                        record->type = _CON_ADD_VAR;
                    } else if (right->type == _CON_ADD_VAR) {
                        if (left->left == right->left && left->right == right->right) {
                            record->left = 0;
                            record->type = _ONE_CON;
                        } else if (left->left == right->left && left->right != right->right) {
                            record->left = left->right;
                            record->right = right->right;
                            record->type = _VAR_SUB_VAR;
                        } else if (left->left != right->left && left->right == right->right) {
                            int cal = left->left - right->left;
                            if (cal >= 0) {
                                record->left = cal;
                                record->type = _ONE_CON;
                            } else {
                                record->left = -cal;
                                record->type = _MINUS_ONE_CON;
                            }
                        }
                    } else if (right->type == _CON_SUB_VAR) {
                        if (left->left == right->left) {
                            record->left = left->right;
                            record->right = right->right;
                            record->type = _VAR_ADD_VAR;
                        }
                    } else if (right->type == _VAR_ADD_CON) {
                        if (left->left == right->right && left->right == right->left) {
                            record->left = 0;
                            record->type = _ONE_CON;
                        } else if (left->left == right->right && left->right != right->left) {
                            record->left = left->right;
                            record->right = right->left;
                            record->type = _VAR_SUB_VAR;
                        } else if (left->left != right->right && left->right == right->left) {
                            int cal = left->left - right->right;
                            if (cal >= 0) {
                                record->left = cal;
                                record->type = _ONE_CON;
                            } else {
                                record->left = -cal;
                                record->type = _MINUS_ONE_CON;
                            }
                        }
                    } else if (right->type == _VAR_SUB_CON) {
                        if (left->right == right->left) {
                            record->left = left->left + right->right;
                            record->type = _ONE_CON;
                        }
                    } else if (right->type == _VAR_ADD_VAR) {
                        if (left->right == right->left) {
                            record->left = left->left;
                            record->right = right->right;
                            record->type = _CON_SUB_VAR;
                        } else if (left->right == right->right) {
                            record->left = left->left;
                            record->right = right->left;
                            record->type = _CON_SUB_VAR;
                        }
                    } else if (right->type == _VAR_SUB_VAR) {
                        if (left->right == right->left) {
                            record->left = left->left;
                            record->right = right->right;
                            record->type = _CON_ADD_VAR;
                        }
                    }
                } else if (left->type == _CON_SUB_VAR) {
                    if (right->type == _ONE_CON) {
                        int cal = left->left - right->left;
                        if (cal > 0) {
                            record->left = cal;
                            record->right = left->right;
                            record->type = _CON_SUB_VAR;
                        } else if (cal == 0) {
                            record->left = left->right;
                            record->type = _MINUS_ONE_VAR;
                        }
                    } else if (right->type == _MINUS_ONE_CON) {
                        record->left = left->left + right->left;
                        record->right = right->right;
                        record->type = _CON_SUB_VAR;
                    } else if (right->type == _MINUS_ONE_VAR) {
                        if (left->right == right->left) {
                            record->left = left->left;
                            record->type = _ONE_CON;
                        }
                    } else if (right->type == _CON_SUB_VAR) {
                        if (left->left == right->left && left->right == right->right) {
                            record->left = 0;
                            record->type = _ONE_CON;
                        } else if (left->left == right->left && left->right != right->right) {
                            record->left = right->right;
                            record->right = left->right;
                            record->type = _VAR_SUB_VAR;
                        } else if (left->left != right->right && left->right == right->right) {
                            int cal = left->left - right->right;
                            if (cal >= 0) {
                                record->left = cal;
                                record->type = _ONE_CON;
                            } else {
                                record->left = -cal;
                                record->type = _MINUS_ONE_CON;
                            }
                        }
                    } else if (left->type == _VAR_ADD_VAR) {
                        if (right->type == _ONE_VAR) {
                            if (left->left == right->left) {
                                record->left = left->right;
                            } else if (left->right == right->left) {
                                record->left = left->left;
                            }
                        } else if (right->type == _CON_ADD_VAR) {
                            if (left->left == right->right) {
                                record->left = left->right;
                                record->right = right->left;
                                record->type = _VAR_SUB_CON;
                            } else if (left->right == right->right) {
                                record->left = left->left;
                                record->right = right->left;
                                record->type = _VAR_SUB_CON;
                            }
                        } else if (right->type == _VAR_ADD_CON) {
                            if (left->left == right->left) {
                                record->left = left->right;
                                record->right = right->right;
                                record->type = _VAR_SUB_CON;
                            } else if (left->right == right->left) {
                                record->left = left->left;
                                record->right = right->right;
                                record->type = _VAR_SUB_CON;
                            }
                        } else if (right->type == _VAR_SUB_CON) {
                            if (left->left == right->left) {
                                record->left = left->right;
                                record->right = right->right;
                                record->type = _VAR_ADD_CON;
                            } else if (left->right == right->left) {
                                record->left = left->left;
                                record->right = right->right;
                                record->type = _VAR_ADD_CON;
                            }
                        } else if (right->type == _VAR_ADD_VAR) {
                            if (left->left == right->left) {
                                record->left = left->right;
                                record->right = right->right;
                                record->type = _VAR_SUB_VAR;
                            } else if (left->right == right->left) {
                                record->left = left->left;
                                record->right = right->right;
                                record->type = _VAR_SUB_VAR;
                            } else if (left->left == right->right) {
                                record->left = left->right;
                                record->right = right->left;
                                record->type = _VAR_SUB_VAR;
                            } else if (left->right == right->right) {
                                record->left = left->left;
                                record->right = right->left;
                                record->type = _VAR_SUB_VAR;
                            }
                        } else if (right->type == _VAR_SUB_VAR) {
                            if (left->left == right->left) {
                                record->left = left->right;
                                record->right = right->right;
                                record->type = _VAR_ADD_VAR;
                            } else if (left->right == right->left) {
                                record->left = left->left;
                                record->right = right->right;
                                record->type = _VAR_ADD_VAR;
                            }
                        }
                    } else if (left->type == _VAR_SUB_VAR) {
                        if (right->type == _ONE_VAR) {
                            if (left->left == right->left) {
                                record->left = left->right;
                                record->type = _MINUS_ONE_VAR;
                            }
                        } else if (right->type == _MINUS_ONE_VAR) {
                            if (left->right == right->left) {
                                record->left = left->left;
                                record->type = _ONE_VAR;
                            }
                        } else if (right->type == _CON_SUB_VAR) {
                            if (left->right == right->right) {
                                record->left = left->left;
                                record->right = right->left;
                                record->type = _VAR_SUB_CON;
                            }
                        } else if (right->type == _VAR_SUB_CON) {
                            if (left->left == right->left) {
                                record->left = right->right;
                                record->right = left->right;
                                record->type = _CON_SUB_VAR;
                            }
                        } else if (right->type == _VAR_SUB_VAR) {
                            if (left->left == right->left && left->right == right->right) {
                                record->left = 0;
                                record->type = _ONE_CON;
                            } else if (left->left == right->left && left->right != right->right) {
                                record->left = right->right;
                                record->right = left->right;
                                record->type = _VAR_SUB_VAR;
                            } else if (left->left != right->left && left->right == right->right) {
                                record->left = left->left;
                                record->right = right->left;
                                record->type = _VAR_SUB_VAR;
                            }
                        }
                    }
                }
                return record;
            }
        case _VAR_DIV_VAR:
            {
                ExpRecord *left = get_exp_record(record->left);
                ExpRecord *right = get_exp_record(record->right);
                if (left == NULL && right == NULL) {
                    if (record->left == record->right) {
                        record->left = 1;
                        record->type = _ONE_CON;
                    }
                    return record;
                } else if (left == NULL && right) {
                    if (right->type == _ONE_CON) {
                        record->right = right->left;
                        record->type = _VAR_DIV_CON;
                    } else if (right->type == _ONE_VAR) {
                        record->right = right->left;
                        record->type = _VAR_DIV_VAR;
                    }
                } else if (left && right == NULL) {
                    if (left->type == _ONE_CON) {
                        record->left = left->left;
                        record->type = _CON_DIV_VAR;
                    } else if (left->type == _ONE_VAR) {
                        record->left = left->left;
                        record->type = _VAR_DIV_VAR;
                    }
                }
                if (left->type == _ONE_CON) {
                    if (right->type == _ONE_CON) {
                        record->left = left->left / right->left;
                        record->type = _ONE_CON;
                    } else if (right->type == _ONE_VAR) {
                        record->left = left->left;
                        record->right = right->left;
                        record->type = _CON_DIV_VAR;
                    } else if (right->type == _MINUS_ONE_CON) {
                        record->left = left->left / right->left;
                        record->type = _MINUS_ONE_CON;
                    } else if (right->type == _CON_DIV_VAR) {
                        record->left = left->left / right->left;
                        record->right = right->right;
                        record->type = _CON_DIV_VAR;
                    }
                } else if (left->type == _ONE_VAR) {
                    if (right->type == _ONE_CON) {
                        record->left = left->left;
                        record->right = right->left;
                    }
                } else if (left->type == _MINUS_ONE_CON) {
                    if (right->type == _ONE_CON) {
                        record->left = left->left / right->right;
                        record->type = _MINUS_ONE_CON;
                    } else if (right->type == _MINUS_ONE_CON) {
                        record->left = left->left / right->right;
                        record->type = _ONE_CON;
                    }
                }
                return record;
            }
    }
    return record;
}

void update_instruct(ExpRecord *record, IRInstruct *instruct) {
    if (record == NULL)
        return;
    instruct->argOne = (int *)malloc(sizeof(int));
    instruct->argTwo = (int *)malloc(sizeof(int));
    *instruct->argOne = record->left;
    *instruct->argTwo = record->right;
    switch(record->type) {
        case _ONE_CON:
            instruct->argOneType = _CONSTANT;
            instruct->type = _ASSIGN;
            break;
        case _ONE_VAR:
            instruct->argOneType = _VAR_OR_TEMP;
            instruct->type = _ASSIGN;
            break;
        case _MINUS_ONE_CON:
            instruct->argOneType = _CONSTANT;
            instruct->type = _MINUS;
            break;
        case _MINUS_ONE_VAR:
            instruct->argOneType = _VAR_OR_TEMP;
            instruct->type = _MINUS;
            break;
        case _CON_ADD_VAR:
            instruct->argOneType = _CONSTANT;
            instruct->argTwoType = _VAR_OR_TEMP;
            instruct->type = _PLUS;
            break;
        case _CON_SUB_VAR:
            instruct->argOneType = _CONSTANT;
            instruct->argTwoType = _VAR_OR_TEMP;
            instruct->type = _SUBSTRACT;
            break;
        case _CON_MUL_VAR:
            instruct->argOneType = _CONSTANT;
            instruct->argTwoType = _VAR_OR_TEMP;
            instruct->type = _MULTIPLY;
            break;
        case _CON_DIV_VAR:
            instruct->argOneType = _CONSTANT;
            instruct->argTwoType = _VAR_OR_TEMP;
            instruct->type = _DIVIDE;
            break;
        case _VAR_ADD_CON:
            instruct->argOneType = _VAR_OR_TEMP;
            instruct->argTwoType = _CONSTANT;
            instruct->type = _PLUS;
            break;
        case _VAR_SUB_CON:
            instruct->argOneType = _VAR_OR_TEMP;
            instruct->argTwoType = _CONSTANT;
            instruct->type = _SUBSTRACT;
            break;
        case _VAR_MUL_CON:
            instruct->argOneType = _VAR_OR_TEMP;
            instruct->argTwoType = _CONSTANT;
            instruct->type = _MULTIPLY;
            break;
        case _VAR_DIV_CON:
            instruct->argOneType = _VAR_OR_TEMP;
            instruct->argTwoType = _CONSTANT;
            instruct->type = _DIVIDE;
            break;
        case _VAR_ADD_VAR:
            instruct->argOneType = _VAR_OR_TEMP;
            instruct->argTwoType = _VAR_OR_TEMP;
            instruct->type = _PLUS;
            break;
        case _VAR_SUB_VAR:
            instruct->argOneType = _VAR_OR_TEMP;
            instruct->argTwoType = _VAR_OR_TEMP;
            instruct->type = _SUBSTRACT;
            break;
        case _VAR_MUL_VAR:
            instruct->argOneType = _VAR_OR_TEMP;
            instruct->argTwoType = _VAR_OR_TEMP;
            instruct->type = _MULTIPLY;
            break;
        case _VAR_DIV_VAR:
            instruct->argOneType = _VAR_OR_TEMP;
            instruct->argTwoType = _VAR_OR_TEMP;
            instruct->type = _DIVIDE;
            break;
    }
}

int verify(int result, ExpRecord *record) {
    int jug = 0;
    switch(record->type) {
        case _ONE_VAR:
        case _MINUS_ONE_VAR:
        case _VAR_ADD_CON:
        case _VAR_SUB_CON:
        case _VAR_DIV_CON:
        case _VAR_MUL_CON:
            if (result == record->left) {
                if (result > 0)
                    varRecords[result - 1] = NULL;
                else
                    tempRecords[result - 1] = NULL;
                jug = 1;
            }
            break;
        case _CON_DIV_VAR:
        case _CON_MUL_VAR:
        case _CON_SUB_VAR:
        case _CON_ADD_VAR:
            if (result == record->right) {
                if (result > 0)
                    varRecords[result - 1] = NULL;
                else
                    tempRecords[result - 1] = NULL;
                jug = 1;
            }
            break;
        case _VAR_ADD_VAR:
        case _VAR_SUB_VAR:
        case _VAR_MUL_VAR:
        case _VAR_DIV_VAR:
            if (result == record->left || result == record->right) {
                if (result > 0)
                    varRecords[result - 1] = NULL;
                else
                    tempRecords[result - 1] = NULL;
                jug = 1;
            }
            break;
    }
    return jug;
}

void propagation_folding_replacement() {
    int i;
    int blocksNum = blocksArrayList->memberNum;
    new_block_container();
    for (i = 0; i < blocksNum; i++) {
        IRBlock *block = blocksArrayList->arr[i];
        int j;
        int instructNum = block->instructNum;
        IRInstruct *instruct = block->instructHead;
        for (j = 1; j < instructNum; j++) {
            int argOne = 0;
            int argOneType;
            if (instruct->argOne) {
                argOne = *instruct->argOne;
                argOneType = instruct->argOneType;
                if (argOneType != _CONSTANT && argOneType != _VAR_OR_TEMP)
                    continue;
            }
            int argTwo = 0;
            int argTwoType;
            if (instruct->argTwo) {
                argTwo = *instruct->argTwo;
                argTwoType = instruct->argTwoType;
                if (argTwoType != _CONSTANT && argTwoType != _VAR_OR_TEMP)
                    continue;
            }
            int result;
            if (instruct->result)
                result = *instruct->result;
            int jug = 0;
            switch(instruct->type) {
                case _ASSIGN:
                case _MINUS:
                {   
                    put_record(result, argOne, argOneType, 0, 0, instruct->type);
                    ExpRecord *record = update_record(result);
                    remove_exp(result);
                    update_instruct(record, instruct);
                    int jug = verify(result, record);
                    ExpRecordNode *re = find_available_exp(record);
                    if (jug == 0 && re && re->result < 0 && result > 0) {
                        re->result = result;
                    }
                    break;
                }
                case _PLUS:
                case _MULTIPLY:
                case _SUBSTRACT:
                case _DIVIDE:
                {
                    put_record(result, argOne, argOneType, argTwo, argTwoType, instruct->type);
                    ExpRecord *record = update_record(result);
                    update_instruct(record, instruct);
                    int jug = verify(result, record);
                    if (jug) {
                        remove_exp(result);
                        break;
                    }
                    switch(record->type) {
                        case _ONE_CON:
                        case _ONE_VAR:
                        case _MINUS_ONE_CON:
                        case _MINUS_ONE_VAR:
                            remove_exp(result);
                            break;
                        default:
                        {
                            ExpRecordNode *re = find_available_exp(record);
                            if (re) {
                                record->left = re->result;
                                record->type = _ONE_VAR;
                            } else {
                                remove_exp(result);
                                ExpRecordNode *node = new_exp_record_node();
                                node->left = record->left;
                                node->right = record->right;
                                node->type = record->type;
                                node->result = result;
                                append_exp_to_list(node);
                            }
                            break;
                        }
                    }
                    
                    break;
                }
                case _RETURN:
                case _WRITE:
                {
                    if (argOneType != _VAR_OR_TEMP)
                        break;
                    ExpRecord *record = update_record(argOne);
                    ExpRecordNode *re = find_available_exp(record);
                    if (re) {
                        record->left = re->result;
                        record->type = _ONE_VAR;
                    }
                    if (record) {
                        switch(record->type) {
                            case _ONE_CON:
                                {
                                    instruct->argOne = (int *)malloc(sizeof(int));
                                    *instruct->argOne = record->left;
                                    instruct->argOneType = _CONSTANT;
                                    break;
                                }
                            case _ONE_VAR:
                                {
                                    instruct->argOne = (int *)malloc(sizeof(int));
                                    *instruct->argOne = record->left;
                                    instruct->argOneType = _VAR_OR_TEMP;
                                    break;
                                }
                        }
                    }
                    break;
                }
                case _READ:
                {
                    ExpRecord *record = update_record(result);
                    ExpRecordNode *re = find_available_exp(record);
                    if (re) {
                        record->left = re->result;
                        record->type = _ONE_VAR;
                    }
                    if (record) {
                        switch(record->type) {
                            case _ONE_VAR:
                                {
                                    instruct->result = (int *)malloc(sizeof(int));
                                    *instruct->result = record->left;
                                    break;
                                }
                        }
                    }
                    break;
                }
            }
            instruct = instruct->next;
        }
        clear_block_container();
        new_block_container();
    }
    free_block_container();
}

typedef struct ArgNode {
    int num;
    struct ArgNode *next;
} ArgNode;

ArgNode *new_arg_node() {
    ArgNode *node = (ArgNode *)malloc(sizeof(ArgNode));
    node->num = 0;
    node->next = NULL;
    return node;
}

void append_arg_to_list(ArgNode **listPointer, int num) {
    ArgNode *list = *listPointer;
    if (list) {
        ArgNode *node = new_arg_node();
        node->next = list;
        node->num = num;
        *listPointer = node;
    } else {
        *listPointer = new_arg_node();
        (*listPointer)->num = num;
    }
}

void remove_arg_from_list(ArgNode **list, int num) {
    ArgNode *node = *list;
    if (node->num == num) {
        *list = node->next;
        return;
    }
    ArgNode *last = node;
    node = node->next;
    while(node) {
        if (node->num == num) {
            last->next = node->next;
            break;
        }
        last = node;
        node = node->next;
    }
}

int check_arg(ArgNode *list, int num) {
    if (list == NULL)
        return 0;
    while(list) {
        if (list->num == num)
            return 1;
        list = list->next;
    }
    return 0;
}

void remove_instruct_from_block(IRBlock *block, IRInstruct *instruct) {
    if (instruct->previous && instruct->next) {
        instruct->previous->next = instruct->next;
        instruct->next->previous = instruct->previous;
        block->instructNum--;
    } else if (instruct->previous) {
        instruct->previous->next = NULL;
        block->instructTail = instruct->previous;
        block->instructNum--;
    } else if (instruct->next) {
        instruct->next->previous = NULL;
        block->instructHead = instruct->next;
        block->instructNum--;
    } else {
        block->instructNum = 0;
        block->instructHead = NULL;
        block->instructTail = NULL;
    }
}

void dead_code_elimination() {
    ArgNode *argList = NULL;
    ArgNode *varList = NULL;
    int i;
    for (i = blocksArrayList->memberNum - 1; i >= 0; i--) {
        IRBlock *block = blocksArrayList->arr[i];
        IRInstruct *instruct = block->instructTail;
        while(instruct) {
            int argOne = 0;
            int argOneType;
            if (instruct->argOne) {
                argOne = *instruct->argOne;
                argOneType = instruct->argOneType;
            }
            int result;
            if (instruct->result)
                result = *instruct->result;
            switch(instruct->type) {
                case _ASSIGN:
                case _MINUS:
                {
                    if (argOneType != _CONSTANT && check_arg(varList, argOne) == 0 && argOne > 0) {
                        append_arg_to_list(&varList, argOne);
                    }
                    break;
                }
                case _PLUS:
                case _MULTIPLY:
                case _SUBSTRACT:
                case _DIVIDE:
                {
                    int argTwo = 0;
                    int argTwoType = -1;
                    if (instruct->argTwo) {
                        argTwo = *instruct->argTwo;
                        argTwoType = instruct->argTwoType;
                    }
                    if (argOneType != _CONSTANT && check_arg(varList, argOne) == 0 && argOne > 0) {
                        append_arg_to_list(&varList, argOne);
                    }
                    if (argTwoType != _CONSTANT && check_arg(varList, argTwo) == 0 && argTwo > 0) {
                        append_arg_to_list(&varList, argTwo);
                    }
                }
                case _COPY:
                {
                    if (argOneType != _CONSTANT && check_arg(varList, argOne) == 0 && argOne > 0) {
                        append_arg_to_list(&varList, argOne);
                    }
                    if (check_arg(varList, result) == 0 && result > 0) {
                        append_arg_to_list(&varList, result);
                    }
                    break;
                }
                case _RETURN:
                case _WRITE:
                case _ARGUMENT:
                {
                    if (argOneType != _CONSTANT && check_arg(varList, argOne) == 0 && argOne > 0) {
                        append_arg_to_list(&varList, argOne);
                    }
                    break;
                }
                case _CALL:
                case _READ:
                {
                    if (check_arg(varList, result) == 0 && result > 0) {
                        append_arg_to_list(&varList, result);
                    }
                    break;
                }
                case _LESSTHAN:
                case _LESSEQUAL:
                case _GREATERTHAN:
                case _GREATEREQUAL:
                case _EQUAL:
                case _NOTEQUAL:
                {
                    int argTwo = 0;
                    int argTwoType = -1;
                    if (instruct->argTwo) {
                        argTwo = *instruct->argTwo;
                        argTwoType = instruct->argTwoType;
                    }
                    if (argOneType != _CONSTANT && check_arg(varList, argOne) == 0 && argOne > 0) {
                        append_arg_to_list(&varList, argOne);
                    }
                    if (argTwoType != _CONSTANT && check_arg(varList, argTwo) == 0 && argTwo > 0) {
                        append_arg_to_list(&varList, argTwo);
                    }
                    break;
                }
            }
            instruct = instruct->previous;
        }
    }
    for (i = blocksArrayList->memberNum - 1; i >= 0; i--) {
        IRBlock *block = blocksArrayList->arr[i];
        IRInstruct *instruct = block->instructTail;
        while(instruct) {
            int argOne = 0;
            int argOneType;
            if (instruct->argOne) {
                argOne = *instruct->argOne;
                argOneType = instruct->argOneType;
            }
            int result;
            if (instruct->result)
                result = *instruct->result;
            switch(instruct->type) {
                case _ASSIGN:
                case _MINUS:
                {
                    if (result < 0) {
                        if (check_arg(argList, result)) {
                            remove_arg_from_list(&argList, result);
                        } else {
                            remove_instruct_from_block(block, instruct);
                            break;
                        }
                    }
                    if (result > 0 && check_arg(varList, result) == 0) {
                        remove_instruct_from_block(block, instruct);
                        break;                          
                    }
                    if (argOneType != _CONSTANT && check_arg(argList, argOne) == 0 && argOne < 0) {
                        append_arg_to_list(&argList, argOne);
                    }
                    break;
                }
                case _PLUS:
                case _MULTIPLY:
                case _SUBSTRACT:
                case _DIVIDE:
                {
                    int argTwo = 0;
                    int argTwoType = -1;
                    if (instruct->argTwo) {
                        argTwo = *instruct->argTwo;
                        argTwoType = instruct->argTwoType;
                    }
                    if (result < 0) {
                        if (check_arg(argList, result)) {
                            remove_arg_from_list(&argList, result);
                        } else {
                            remove_instruct_from_block(block, instruct);
                            break;
                        }
                    }
                    if (result > 0 && check_arg(varList, result) == 0) {
                        remove_instruct_from_block(block, instruct);
                        break;                          
                    }
                    if (argOneType != _CONSTANT && check_arg(argList, argOne) == 0 && argOne < 0) {
                        append_arg_to_list(&argList, argOne);
                    }
                    if (argTwoType != _CONSTANT && check_arg(argList, argTwo) == 0 && argTwo < 0) {
                        append_arg_to_list(&argList, argTwo);
                    }
                }
                case _COPY:
                {
                    if (argOneType != _CONSTANT && check_arg(argList, argOne) == 0 && argOne < 0) {
                        append_arg_to_list(&argList, argOne);
                    }
                    if (check_arg(argList, result) == 0 && result < 0) {
                        append_arg_to_list(&argList, result);
                    }
                    break;
                }
                case _RETURN:
                case _WRITE:
                case _ARGUMENT:
                {
                    if (argOneType != _CONSTANT && check_arg(argList, argOne) == 0 && argOne < 0) {
                        append_arg_to_list(&argList, argOne);
                    }
                    break;
                }
                case _CALL:
                case _READ:
                {
                    if (check_arg(argList, result) == 0 && result < 0) {
                        append_arg_to_list(&argList, result);
                    }
                    break;
                }
                case _LESSTHAN:
                case _LESSEQUAL:
                case _GREATERTHAN:
                case _GREATEREQUAL:
                case _EQUAL:
                case _NOTEQUAL:
                {
                    int argTwo = 0;
                    int argTwoType = -1;
                    if (instruct->argTwo) {
                        argTwo = *instruct->argTwo;
                        argTwoType = instruct->argTwoType;
                    }
                    if (argOneType != _CONSTANT && check_arg(argList, argOne) == 0 && argOne < 0) {
                        append_arg_to_list(&argList, argOne);
                    }
                    if (argTwoType != _CONSTANT && check_arg(argList, argTwo) == 0 && argTwo < 0) {
                        append_arg_to_list(&argList, argTwo);
                    }
                    break;
                }
            }
            instruct = instruct->previous;
        }
    }
}