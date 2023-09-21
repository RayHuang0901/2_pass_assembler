#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int str_to_hex(char c[]){  		//字串轉十六 
	int num;
	sscanf(c,"%X",&num);
	return num;
}
int str_to_int(char c[]){		//字串轉十 
	int num;
	sscanf(c,"%d",&num);
	return num;
}
int get_opcode(char c[]){		//得到opcode
	FILE *opcode=fopen("opcode.txt","r");
	char op[10];
	int num;
	while(1){
		fscanf(opcode,"%s %X",op,&num);
		if(strcmp(op,c)==0){
			fclose(opcode);
			return num;
		}
	}
}
int get_pos(char c[]){			//得到label的值
	FILE *symbol=fopen("symbol.txt","r");
	char sym[10];
	int num;
	while(1){
		fscanf(symbol,"%s %X",sym,&num);
		if(strcmp(sym,c)==0){
			fclose(symbol);
			return num;
		}
	}
}
int get_object(char first[],char second[]){			//算出object code
	int x=0,opnum,posnum;
	char *temp;
	opnum=get_opcode(first);
	if(strcmp(first,"RSUB")==0){
		return opnum*65536;
	}
	if(strstr(second,",")){
		temp=strtok(second,",");
		x=1;
	}
	else{
		temp=second;
	}
	posnum=get_pos(temp);
	return opnum*65536+x*4096*8+posnum;
}
int main(){
	FILE *source=fopen("source.txt","r");
	FILE *locate=fopen("locate.txt","w");
	FILE *symbol=fopen("symbol.txt","w");
	FILE *output=fopen("output.txt","w");
	FILE *program=fopen("program.txt","w");
	int i,loc,object,fir_pos,len=0,b=0,fst=1,count=0,sum=0;
	char name[6],first[10],second[10],third[10],fourth[10],c,temp[10],now[100];
	fscanf(source,"%s",name);
	fscanf(source,"%s",first);
	fscanf(source,"%s",second);
	loc=str_to_hex(second);
	fir_pos=loc;
	fprintf(locate,"%04X\t%s\t%s\t%s\n",loc,name,first,second);
	while(1){					//pass 1
		fscanf(source,"%s",first);
		if(strcmp(first,"END")==0){
			fscanf(source,"%s",second);
			fprintf(locate,"\t\tEND\t%s",second);
			break;;
		}
		else if(strcmp(first,"RSUB")==0){
			fprintf(locate,"%04X\t\tRSUB\n",loc);
			loc+=3;
			continue;
		}
		else{
			fscanf(source,"%s",second);
			fscanf(source,"%c",&c);
			if(c=='\n'){
				fprintf(locate,"%04X\t\t%s\t%s\n",loc,first,second);
				loc+=3;
			}
			else{
				fscanf(source,"%s",third);
				fprintf(symbol,"%s\t%X\n",first,loc);			//symbol table 
				if(strcmp(second,"BYTE")==0){
					if(third[0]=='X'){
						fprintf(locate,"%04X\t%s\t%s\t%s\n",loc,first,second,third);
						loc+=(strlen(third)-3)/2;
					}
					else if(third[0]=='C'){
						//printf("in");
						fprintf(locate,"%04X\t%s\t%s\t%s\n",loc,first,second,third);
						loc+=strlen(third)-3;
					}
				}
				else if(strcmp(second,"RESW")==0){
					fprintf(locate,"%04X\t%s\t%s\t%s\n",loc,first,second,third);
					loc+=3*str_to_int(third);
				}
				else if(strcmp(second,"RESB")==0){
					fprintf(locate,"%04X\t%s\t%s\t%s\n",loc,first,second,third);
					loc+=str_to_int(third);
				}
				else{
					fprintf(locate,"%04X\t%s\t%s\t%s\n",loc,first,second,third);
					loc+=3;
				}
			}
		}
	}
	fclose(source);
	fclose(locate);
	fclose(symbol);
	locate=fopen("locate.txt","r");
	fscanf(locate,"%s",first);
	fscanf(locate,"%s",second);
	fscanf(locate,"%s",third);
	fscanf(locate,"%s",fourth);
	fprintf(output,"%-7s\t%-7s\t%-7s\t%-7s\n",first,second,third,fourth);
	fprintf(program,"H^%-6s^%06X^%06X\n",name,fir_pos,loc-fir_pos);
	loc=str_to_hex(first);
	while(1){					//pass2
		fscanf(locate,"%s",first);
		fscanf(locate,"%s",second);
		if(strcmp(first,"END")==0){
			fprintf(output,"\t\t%-7s\t%-7s",first,second);
			break;
		}
		else if(strcmp(second,"RSUB")==0){
			object=get_object(second,"");
			fprintf(output,"%-7s\t\t\t%-7s\t\t\t%X\n",first,second,object);
		}
		else{
			fscanf(locate,"%s",third);
			fscanf(locate,"%c",&c);
			if(c=='\n'){
				object=get_object(second,third);
				fprintf(output,"%-7s\t\t\t%-7s\t%-7s\t%06X\n",first,second,third,object);	
			}
			else{
				fscanf(locate,"%s",fourth);
				if(strcmp(third,"BYTE")==0){
					int bytelen=strlen(fourth)-3;
					if(fourth[0]=='X'){
						sscanf(fourth,"%c'%X'",&c,&object);
						fprintf(output,"%-7s\t%-7s\t%-7s\t%-7s\t%0*X\n",first,second,third,fourth,bytelen,object);
						
					}
					else if(fourth[0]=='C'){
						sscanf(fourth,"%c'%s'",&c,temp);
						FILE *f=fopen("test.txt","w+");
						for(i=0;i<strlen(temp)-1;i++){
							fprintf(f,"%X",temp[i]);
						}
						fseek(f,0,0);
						fscanf(f,"%s",temp);
						sscanf(temp,"%X",&object);
						fclose(f);
						remove("test.txt");
						fprintf(output,"%-7s\t%-7s\t%-7s\t%-7s\t%0*X\n",first,second,third,fourth,bytelen,object);
					}
				}
				else if(strcmp(third,"RESW")==0){
					fprintf(output,"%-7s\t%-7s\t%-7s\t%-7s\n",first,second,third,fourth);
					b=1;
				}
				else if(strcmp(third,"RESB")==0){
					fprintf(output,"%-7s\t%-7s\t%-7s\t%-7s\n",first,second,third,fourth);
					b=1;
				}
				else if(strcmp(third,"WORD")==0){
					object=str_to_int(fourth);
					fprintf(output,"%-7s\t%-7s\t%-7s\t%-7s\t%06X\n",first,second,third,fourth,object);
				}
				else{
					object=get_object(third,fourth);
					fprintf(output,"%-7s\t%-7s\t%-7s\t%-7s\t%06X\n",first,second,third,fourth,object);
					
				}
			}
		}
		if(b==1 && len==0){     // 印出object program 
			b=0;
			continue;
		}
		if(b==1 && len!=0){
			now[len]='\0';
			fprintf(program,"^%02X%s\n",sum,now);
			now[0]='\0';
			sum=0;
			len=0;
			b=0;
			continue;
		}
		char str[7];
		if(strcmp(third,"BYTE")==0){
			if(fourth[0]=='X')
				count=strlen(fourth)-3;
			else
				count=(strlen(fourth)-3)*2;
			sprintf(str,"%0*X",count,object);
		}
		else{
			count=6;
			sprintf(str,"%0*X",count,object);
		}
		
		if(len==0){
			fprintf(program,"T^%06X",str_to_hex(first));
			now[0]='\0';
		}
		if(len>70-strlen(str)-1){
			now[len]='\0';
			fprintf(program,"^%02X%s\n",sum,now);
			fprintf(program,"T^%06X",str_to_hex(first));
			sum=0;
			now[0]='\0';
			len=0;
		}
		strcat(now,"^");
		strcat(now,str);
		sum+=count/2;
		len+=strlen(str)+1;
		
		
	}
	
	fprintf(program,"^%02X%s\n",sum,now);
	fprintf(program,"E^%06X\n",fir_pos);
	return 0;
}

