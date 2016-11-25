//
//  main.cpp
//  mysql
//
//  Created by 冯文斌 on 16/10/14.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#include <iostream>

#include "UTFail.h"
#include "TestDatabase.h"

using namespace std;
using namespace MySQLWrap;


int main(int argc, const char * argv[]) {
    
    TestDatabase testDatabase;
    int failures = testDatabase.RunTests(false);
    
    cout << "failures:" << failures << endl;
    return 0;
}
