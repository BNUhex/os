#include "filesys.h"

/*
    查看当前用户信息
*/

//function02_03：打印当前用户的id号、登录密码、所属用户组
void who(int user_id){
    printf("User_id\t\tPassword\tGroup_id\n");
    for (int i = 0; i < PWDNUM; i++){
        if (pwd[i].p_uid == user[user_id].u_uid){
            printf("%hu\t\t%s\t\t%hu\n", pwd[i].p_uid, pwd[i].password, pwd[i].p_gid);
        }
    }
}
