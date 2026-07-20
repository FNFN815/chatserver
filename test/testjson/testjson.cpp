#include <nlohmann/json.hpp>
using json = nlohmann::json;
#include<iostream>
#include<vector>
#include<map>
#include<string>
using namespace std;
//json示例1
string func1()
{
    json js;
    js["msg_type"]=2;
    js["from"]="zhang san";
    js["to"]="li si";
    js["msg"]="hello,what are you doing now?";
    //cout<<js<<endl;
    string sendbuf=js.dump();
    //cout<<senbnuf<<endl;
    return sendbuf;
}
//json 示例2
string func2()
{
    json js;
    //添加数组
    js["id"]={1,2,3,4,5};
    js["name"]="zhang san";
    //添加对象
    js["msg"]["zhang san"]="hell world";
    js["msg"]["li su"]="hello,China";
    //上面俩个等同于
    js["msg"]={{"zhang san","hello world"},{"li su","hello,China"}};
    //cout<<js<<endl;
    return js.dump();

}
//json 序列化示例3
string func3()
{
    json js;
    //序列化vector
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    js["list"]=vec;
    //序列化map
    map<int,string> m;
    m.insert({1,"黄山"});
    m.insert({2,"泰山"});
    m.insert({3,"华山"});
    js["path"]=m;
    string sendbuf=js.dump();
    //cout<<sendbuf<<endl;
    return sendbuf;
}
int main()
{   
    string recvbuf=func3();
    //反序列化
     json jsbuf=json::parse(recvbuf);
     //1.
    // cout<<jsbuf["msg_type"]<<endl;
    // cout<<jsbuf["msg"]<<endl;
    // cout<<jsbuf["to"]<<endl;
    // cout<<jsbuf["from"]<<endl;
    // cout<<jsbuf["id"]<<endl;
    // auto arr=jsbuf["id"];
    // cout<<arr[0]<<endl;
    
    //2.
    // auto msgjs=jsbuf["msg"];
    // cout<<msgjs["zhang san"]<<endl;
    // cout<<msgjs["li su"]<<endl;

    //3.
    vector<int> vec=jsbuf["list"];
    for(auto &item:vec)
    {
        cout<<item<<endl;
    }
    map<int,string> m=jsbuf["path"];
    for(auto& item:m)
    {
        cout<<item.first<<" "<<item.second<<endl;
    }
    return 0;
}