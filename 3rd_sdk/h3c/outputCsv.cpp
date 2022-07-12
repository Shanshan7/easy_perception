#include"outputCsv.h"

void outputCsv(InformationSaveAndOutput edge)
{
   std::cout<<"解析开始了 is "<<edge.ones.ageGroup<<std::endl;  
            std::ofstream outFile;
            std::ofstream outFile1;
            std::ofstream outFile2;
            long long  event_type=edge.event_type;
            int          sex;
            char mystr[25]={0};
            if(33751045==event_type){
                sex=edge.person_Fs.sex;
            }
            if(33751046==event_type){
                sex=edge.person_Fc.sex;
            }
            if(33751047==event_type){
                sex=edge.ones.sex;
            }

            std::string  sex1="."; 
            std::string  glasses1 =".";
            std::string  cap1=".";
            std::string  respitator1=".";
            std::string  name=edge.person_Fc.name;
            int          nativeCity=edge.person_Fc.nativeCity;
            std::string  bronDate=edge.person_Fc.bronDate;
            int          idType = edge.person_Fc.idType;
            std::string  idNumber=edge.person_Fc.idNumber;
            long long    similarity=edge.person_Fc.similarity;
            std::string  dev_id=edge.cammers.dev_id;
            std::string  dev_ip=edge.cammers.dev_ip;
            
            long long time =0;
            std::string ageGroup ;//年龄
            std::string hairStyle;//发型
            std::string coatcolor;//上身颜色
            std::string trousersColor;//下身颜色
            std::string  Orientation;//朝向

            //+++++++++++++++++++++++++++++++++++++++++++++++++++++
            if (1==sex)
            {
                sex1="male";
            }
            if (2==sex)
            {
                sex1="famale";
            }
            if(0==sex||9==sex)
            {
                sex1="unknow";
            }
            //+++++++++++++++++++++++++++++++++++++++++++++++++++++++
            if(0==edge.person_Fs.glasses)
            {
                glasses1="NO";
            }
            if(1==edge.person_Fs.glasses)
            {
                glasses1="YES";
            }
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            if(0==edge.person_Fs.cap)
            {
                cap1="NO";
            }
            if(1==edge.person_Fs.cap)
            {
                cap1="YES";
            }
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            if(0==edge.person_Fs.respitator)
            {
                respitator1="NO";
            }
            if(1==edge.person_Fs.respitator)
            {
                respitator1="YES";
            }
            //hairStyle=edge.ones.hairStyle;
            switch (edge.ones.coatcolor)
            {
            case 1 :coatcolor="黑";break;
            case 2 :coatcolor="白";break;
            case 3 :coatcolor="灰";break;
            case 4 :coatcolor="红";break;
            case 5 :coatcolor="蓝";break;
            case 6 :coatcolor="黄";break;
            case 7 :coatcolor="橙";break;
            case 8 :coatcolor="棕";break;
            case 9 :coatcolor="绿";break;
            case 10 :coatcolor="紫";break;
            case 11 :coatcolor="青";break;
            case 12 :coatcolor="粉";break;
            case 13 :coatcolor="透明";break;
            case 80 :coatcolor="黄绿";break;
            case 81 :coatcolor="渐变绿";break;
            case 82 :coatcolor="金";break;
            case 83 :coatcolor="银";break;
            case 99 :coatcolor="其他";break;

            
            default:
                break;
            }

            switch (edge.ones.ageGroup)
            {
            
            case 1:ageGroup="婴幼儿";  break;
            case 2:ageGroup="儿童";  break;
            case 3:ageGroup="少年";  break;
            case 4:ageGroup="青少年";  break;
            case 5:ageGroup="青年";  break;
            case 6:ageGroup="壮年";  break;
            case 7:ageGroup="中年"; 
            std::cout<<"解析开始了"<<std::endl;
            std::cout<<edge.ones.ageGroup; 
            break;
            case 8:ageGroup="中老年";  break;
            case 9:ageGroup="老年";  break;
            case 99:ageGroup="其他";  break;    
            default:
                break;
            }
            switch (edge.ones.hairlen)
            {
            case 0:hairStyle="长发";break;
            case 1:hairStyle="短发";break;
            case 2:hairStyle="无发";break;
            case 3:hairStyle="未识别";break;       
            default:
                break;
            }
            switch (edge.ones.trousersColor)
            {
            case 1 :trousersColor="黑";break;
            case 2 :trousersColor="白";break;
            case 3 :trousersColor="灰";break;
            case 4 :trousersColor="红";break;
            case 5 :trousersColor="蓝";break;
            case 6 :trousersColor="黄";break;
            case 7 :trousersColor="橙";break;
            case 8 :trousersColor="棕";break;
            case 9 :trousersColor="绿";break;
            case 10 :trousersColor="紫";break;
            case 11 :trousersColor="青";break;
            case 12 :trousersColor="粉";break;
            case 13 :trousersColor="透明";break;
            case 80 :trousersColor="黄绿";break;
            case 81 :trousersColor="渐变绿";break;
            case 82 :trousersColor="金";break;
            case 83 :trousersColor="银";break;
            case 99 :trousersColor="其他";break;

            
            default:
                break;
            }
            if(1==edge.ones.Orientation){
                Orientation="正面";
            }
            else if(2==edge.ones.Orientation){
                Orientation="背面";
            }
            else{
                Orientation="其他";
            }
            //________________________________________________________
            
            //outFile<<"抓拍时间"<<','<<"性别"<<','<<"眼镜"<<','<<"帽子"<<','<<"口罩"<<','<<std::endl;
           
            //outFile1<<"抓拍时间"<<','<<"姓名"<<','<<"性别"<<','<<"籍贯"<<','<<"出生日期"<<','<<"证件类型"<<','<<"证件号"<<','<<"设备ID"<<','<<"设备IP"<<std::endl;
            
            if(33751045==edge.event_type){
               outFile.open("data.csv",std::ios::app);
               std:: cout<<"wenjian yi dakai"<<std::endl;
               if(outFile.is_open()){
                   std::cout<<"face_S_success";
                   outFile<<edge.person_Fs.mystr<<','<<sex1<<','<<glasses1<<','<<cap1<<','<<respitator1<<','<<std::endl;
               }
               outFile.close();
            }
            if(33751046==edge.event_type){
                if(outFile1.is_open()){
                   outFile1.open("Face_recognition.csv",std::ios::app);
                   std::cout<<"face_R_success"<<std::endl;
                   outFile1<<edge.person_Fc.mystr<<','<<name<<','<<sex1<<','<<nativeCity<<','<<bronDate<<','<<idType<<','<<idNumber<<','<<dev_id<<','<<dev_ip<<std::endl;
                   outFile1.close();
                }
                
            } 
            if(33751047==edge.event_type){
                outFile2.open("person_Features.csv",std::ios::app);
                outFile2<<sex1<<','<<ageGroup<<','<<coatcolor<<','<<trousersColor<<','<<hairStyle<<','<<Orientation<<std::endl;
                outFile2.close();

            }

            

}