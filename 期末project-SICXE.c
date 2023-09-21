#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int str_to_hex(char c[]){			//字串轉十六 
	int num;
	sscanf(c,"%X",&num);
	return num;
}
int str_to_int(char c[]){			//字串轉十 
	int num;
	sscanf(c,"%d",&num);
	return num;
}
int get_opcode(char c[]){			//得到opcode 
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
int get_pos(char c[]){				//得到label的值 
	FILE *symbol=fopen("symbol.txt","r");
	char sym[10];
	int num;
	//printf("%s",c);
	while(!feof(symbol)){
		fscanf(symbol,"%s %X",sym,&num);
		if(strcmp(sym,c)==0){
			fclose(symbol);
			return num;
		}
	}
	return -1;
}
unsigned int get_object(char first[],char second[],int base,int pc){   //得到object的值 
	int x=0,opnum,posnum;
	char *temp;
	
	if(first[0]=='+'){								//format 4 處理 
		temp=strtok(first,"+");
		opnum=get_opcode(temp);
		if(second[0]=='#'){
			temp=strtok(second,"#");
			posnum=str_to_int(temp);
			return (opnum+1)*16777216+posnum+1048576;
		}
		posnum=get_pos(second);
		return (opnum+3)*16777216+posnum+1048576;
	}
	opnum=get_opcode(first);
	if(strcmp(first,"RSUB")==0){
		return (opnum+3)*65536;
	}
	if(strcmp(first,"TIXR")==0 || strcmp(first,"CLEAR")==0 ){   //處理暫存器 
		switch(second[0]){
			case 'A':
				return opnum*256;
			case 'X':
				return opnum*256+1*16;
			case 'L':
				return opnum*256+2*16;
			case 'B':
				return opnum*256+3*16;
			case 'S':
				if(strlen(second)==2)
					return opnum*256+9*16;
				return opnum*256+4*16;
			case 'T':
				return opnum*256+5*16;
			case 'F':
				return opnum*256+6*16;
			case 'P':
				return opnum*256+8*16;
		}
	}
	if(strcmp(first,"COMPR")==0){								//處理暫存器
		char strtemp[10];
		strcpy(strtemp,second);
		temp=strtok(strtemp,",");
		//printf("%X",opnum);
		switch(temp[0]){
			case 'A':
				opnum*=256;
				break;
			case 'X':
				opnum=opnum*256+1*16;
				break;
			case 'L':
				opnum=opnum*256+2*16;
				break;
			case 'B':
				opnum=opnum*256+3*16;
				break;
			case 'S':
				if(strlen(temp)==2)
					opnum=opnum*256+9*16;
				else
					opnum=opnum*256+4*16;
				break;
			case 'T':
				opnum=opnum*256+5*16;
				break;
			case 'F':
				opnum=opnum*256+6*16;
				break;
			case 'P':
				opnum=opnum*256+8*16;
				break;
		}
		temp=strtok(NULL,",");
		
		switch(temp[0]){
			case 'A':
				return opnum;
			case 'X':
				return opnum+=1;
			case 'L':
				return opnum+=2;
			case 'B':
				return opnum+=3;
			case 'S':
				if(strlen(temp)==2)
					return opnum+=9;
				return opnum+=4;
			case 'T':
				return opnum+=5;
			case 'F':
				return opnum+=6;
			case 'P':
				return opnum+=8;
		}
	}
	if(second[0]=='#'){							//處理立即值 
		temp=strtok(second,"#");
		int i=get_pos(temp);
		if(i==-1){
			posnum=str_to_int(temp);
			return (opnum+1)*65536+posnum;
		}
		posnum=i;
		posnum-=pc+3;
		if(-2048<=posnum && posnum<=2047){
			posnum=posnum&4095;
			return (opnum+1)*65536+2*4096+posnum;
		}
		posnum=i;
		posnum-=base;
		posnum=posnum&4095;
		return (opnum+1)*65536+4*4096+posnum;
	}
	if(second[0]=='@'){						//處理indirect 
		temp=strtok(second,"@");
		int i=get_pos(temp);
		posnum=i;
		posnum-=pc+3;
		if(-2048<=posnum && posnum<=2047){
			posnum=posnum&4095;
			return (opnum+2)*65536+2*4096+posnum;
		}
		posnum=i;
		posnum-=base;
		posnum=posnum&4095;
		return (opnum+2)*65536+4*4096+posnum;
	}
	if(strstr(second,",")){						//處理index addressing 
		char strtemp[15];
		strcpy(strtemp,second);
		temp=strtok(strtemp,",");
		int i=get_pos(temp);
		posnum=i;
		posnum-=pc+3;
		if(-2048<=posnum && posnum<=2047){
			posnum=posnum&4095;
			return (opnum+3)*65536+10*4096+posnum;
		}
		posnum=i;
		posnum-=base;
		posnum=posnum&4095;
		return (opnum+3)*65536+12*4096+posnum;
	}
	else{										//處理sample 
		temp=second;
		int i=get_pos(temp);
		posnum=i;
		posnum-=pc+3;
		if(-2048<=posnum && posnum<=2047){
			posnum=posnum&4095;
			return (opnum+3)*65536+2*4096+posnum;
		}
		posnum=i;
		posnum-=base;
		posnum=posnum&4095;
		return (opnum+3)*65536+4*4096+posnum;
	}
}
int main(){
	FILE *source=fopen("SICXEsource.txt","r");
	FILE *locate=fopen("locate.txt","w");
	FILE *symbol=fopen("symbol.txt","w");
	FILE *output=fopen("output.txt","w");
	FILE *program=fopen("program.txt","w");
	int i,loc,fir_pos,len=0,b=0,fst=1,count=0,sum=0,base,index=0;
	unsigned int object;
	char name[6],first[10],second[10],third[10],fourth[10],c,temp[10],now[100],format4[10][12];
	fscanf(source,"%s",name);
	fscanf(source,"%s",first);
	fscanf(source,"%s",second);
	loc=str_to_hex(second);
	fir_pos=loc;
	fprintf(locate,"%04X\t%s\t%s\t%s\n",loc,name,first,second);
	while(1){								//pass1
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
		else if(strcmp(first,"BASE")==0){
			fscanf(source,"%s",second);
			fprintf(locate,"\t\tBASE\t%s\n",second);
			continue;
		}
		else if(strcmp(first,"TIXR")==0){
			fscanf(source,"%s",second);
			fprintf(locate,"%04X\t\tTIXR\t%s\n",loc,second);
			loc+=2;
			continue;
		}
		else if(strcmp(first,"CLEAR")==0){
			fscanf(source,"%s",second);
			fprintf(locate,"%04X\t\tCLEAR\t%s\n",loc,second);
			loc+=2;
			continue;
		}
		else if(strcmp(first,"COMPR")==0){
			fscanf(source,"%s",second);
			fprintf(locate,"%04X\t\tCOMPR\t%s\n",loc,second);
			loc+=2;
			continue;
		}
		else{
			fscanf(source,"%s",second);
			fscanf(source,"%c",&c);
			if(c=='\n'){
				fprintf(locate,"%04X\t\t%s\t%s\n",loc,first,second);     
				if(first[0]=='+'){
					loc+=4;
				}
				else
					loc+=3;
			}
			else{
				fscanf(source,"%s",third);
				//printf("%s\n",third);
				fprintf(symbol,"%s\t%04X\n",first,loc);         //symbol table
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
				else if(strcmp(second,"CLEAR")==0){
					fprintf(locate,"%04X\t%s\tCLEAR\t%s\n",loc,first,third);
					loc+=2;
					continue;
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
					if(second[0]=='+'){
						loc+=4;
					}
					else
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
	while(1){								//pass 2
		
		fscanf(locate,"%s",first);
		fscanf(locate,"%s",second);
		if(strcmp(first,"END")==0){
			fprintf(output,"\t\t%-7s\t%-7s",first,second);
			break;
		}
		else if(strcmp(second,"RSUB")==0){
			object=get_object(second,"",base,str_to_hex(first));
			fprintf(output,"%-7s\t\t\t%-7s\t\t\t%X\n",first,second,object);
		}
		else if(strcmp(first,"BASE")==0){
			base=get_pos(second);
			fprintf(output,"\t\tBASE\t\t%s\n",second);
			continue;
		}
		else if(strcmp(second,"TIXR")==0){
			fscanf(locate,"%s",third);
			object=get_object(second,third,base,str_to_hex(first));
			fprintf(output,"%-7s\t\t\tTIXR\t\t%-7s\t%04X\n",first,third,object);
		}
		else if(strcmp(second,"CLEAR")==0){
			fscanf(locate,"%s",third);
			object=get_object(second,third,base,str_to_hex(first));
			fprintf(output,"%-7s\t\t\tCLEAR\t\t%-7s\t%04X\n",first,third,object);
		}
		else if(strcmp(second,"COMPR")==0){
			fscanf(locate,"%s",third);
			object=get_object(second,third,base,str_to_hex(first));
			fprintf(output,"%-7s\t\t\tCOMPR\t\t%-7s\t%04X\n",first,third,object);
		}
		else{
			fscanf(locate,"%s",third);
			fscanf(locate,"%c",&c);
			if(c=='\n'){
				object=get_object(second,third,base,str_to_hex(first));
				fprintf(output,"%-7s\t\t\t%-7s\t%-7s\t%06X\n",first,second,third,object);	
			}
			else{
				fscanf(locate,"%s",fourth);
				int bytelen=strlen(fourth)-3;
				if(strcmp(third,"BYTE")==0){
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
				else if(strcmp(third,"CLEAR")==0){
					object=get_object(third,fourth,base,str_to_hex(first));
					fprintf(output,"%-7s\t%-7s\tCLEAR\t\t%-7s\t%04X\n",first,second,fourth,object);
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
					object=get_object(third,fourth,base,str_to_hex(first));
					fprintf(output,"%-7s\t%-7s\t%-7s\t%-7s\t%06X\n",first,second,third,fourth,object);
					
				}
			}
		}
		if(b==1 && len==0){			// 印出object program
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
		else if(third[0]=='+'||second[0]=='+'){
			count=8;
			sprintf(str,"%0*X",count,object);
			if(third[0]!='#'&&fourth[0]!='#')
				sprintf(format4[index++],"M^%06X^05",str_to_hex(first+1));
		}
		else if(strcmp(second,"TIXR")==0 ||strcmp(second,"CLEAR")==0||strcmp(second,"COMPR")==0||strcmp(third,"CLEAR")==0){
			count=4;
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
	for(i=0;i<index;i++){
		fprintf(program,"%s\n",format4[i]);
	}
	fprintf(program,"E^%06X\n",fir_pos);
	return 0;
}

