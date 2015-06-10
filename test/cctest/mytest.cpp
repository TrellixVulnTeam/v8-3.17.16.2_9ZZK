


#include "v8.h"  
#include "utils.h"  
  
#include <iostream>  
#include <string>  
  
using namespace std;  
  
using namespace v8;  
  
enum AppState
{  
    IDEL = 0,  
    LOADED,  
    STOP  
};  
  
class CloudApp 
{  
public:  
	CloudApp* CreateApp();
	CloudApp()
	{
		state = IDEL;
		appId = 0;
	}
    CloudApp(int id)
	{   
        state = IDEL;  
        appId = id;  
    }  
	~CloudApp()
	{
		int i = 0;
	}

    void start()
	{  
        cout << "CloudApp been Loaded id = " << appId << endl;  
        state = LOADED;  
    };  
	bool IsEqual(CloudApp * app)
	{
		if ( app->appId == this->appId )
		{
			return true;
		}
		return false;
	}
  
    int getState() { return state;}  
    int getAppId() { return appId;}
      
private:  
    AppState state;  
    int appId;    
};  
  
//��MakeWeakע���callback.  
void CloudAppWeakReferenceCallback(Persistent<Value> object, void * param)
{  
    if (CloudApp* cloudapp = static_cast<CloudApp*>(param))
	{  
        delete cloudapp;  
    }  
}  
  
//��C++ָ��ͨ��External����ΪPersistent���󣬱����ָ�뱻����  
Handle<External> MakeWeakCloudApp(void* parameter) 
{  
    Persistent<External> persistentCloudApp = Persistent<External>::New(External::New(parameter));  
          
//MakeWeak�ǳ���Ҫ����JS����newһ��CloudApp����֮��  
//C++Ҳ����newһ����Ӧ��ָ�롣  
//JS��������֮�������취ȥ����C++��ָ�룬����ͨ��MakeWeak��ʵ�֣�  
//MakeWeak����ҪĿ����Ϊ�˼��Persistent Handle���˵�ǰPersistent   
//��Ψһ�����⣬û�����������ã��Ϳ����������Persistent Handle�ˣ�  
//ͬʱ����MakeWeak��callback���������ǿ��������callback��delete   
//C++ָ��  
    persistentCloudApp.MakeWeak(parameter, CloudAppWeakReferenceCallback);  
  
    return persistentCloudApp;  
}  
  
//��JS�������Ĳ�������֮�󣬴���C++����  
CloudApp* NewCloudApp(const Arguments& args)
{  
    CloudApp* cloudApp = NULL;  
    if (args.Length() == 1) 
	{  
        cloudApp = new CloudApp(args[0]->ToInt32()->Value());   
    } 
	else
	{  
        v8::ThrowException(String::New("Too many parameters for NewCloudApp"));  
    }  
  
    return cloudApp;  
}  
  
//�൱��JS��Ӧ�Ĺ��캯������JS��ʹ��new CloudApp��ʱ�����callback���Զ�������  
Handle<Value> CloudAppConstructCallback(const Arguments& args)
{  
    if (!args.IsConstructCall())  
        return Undefined();  
    CloudApp* cloudapp = NewCloudApp(args);  
    Handle<Object> object = args.This();  
    object->SetInternalField(0, MakeWeakCloudApp(cloudapp));  
    return Undefined();  
}  
 
Handle<Value> JS_CreateApp(const Arguments &args)
{
	CloudApp* cloudapp = NewCloudApp(args);
	Handle<Object> object = args.This();
	object->SetInternalField(0, MakeWeakCloudApp(cloudapp));
	return object;
}

Handle<Value> JS_GetState(const Arguments& args) {  
    Handle<Object> self = args.Holder();  
  
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));  
    void* ptr = wrap->Value();  
    CloudApp* cloudapp = static_cast<CloudApp*>(ptr);  
 
    return Integer::New(cloudapp->getState());  
}  
  
Handle<Value> JS_GetAppId(const Arguments& args) {  
    Handle<Object> self = args.Holder();  
  
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));  
    void* ptr = wrap->Value();  
    CloudApp* cloudapp = static_cast<CloudApp*>(ptr);  
  
    return Integer::New(cloudapp->getAppId());  
}   

Handle<Value> JS_IsEqual(const Arguments &args)
{
	Handle<Object> self = args.Holder();
	Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
	void* ptr = wrap->Value();
	CloudApp* cloudapp = static_cast<CloudApp*>(ptr);
	
	if ( args[0]->IsArray() )
	{
		int i = args.Length();
		i = 0;
	}
	Handle<Array> arr = Handle<Array>::Cast(args[0]);
	int size = arr->Length();
	CloudApp app[2];
	for (int i = 0; i < size; i++ )
	{
		Handle<Value> va = arr->Get(Integer::New(i));
		Handle<External> field = Handle<External>::Cast(va->ToObject()->GetInternalField(0));
		void *obj_ptr = field->Value();
		CloudApp *p = static_cast<CloudApp*>(obj_ptr);
		app[i] = *p;
	}

	cloudapp->IsEqual(app);

	//int ilen = va->InternalFieldCount();
	//Local<External> clwrap = Local<External>::Cast(va->GetInternalField(0));

	//CloudApp *p = (CloudApp*)args[0]->Int32Value();
	return Boolean::New(false);
}
  
Handle<Value> JS_Start(const Arguments& args) {  
    Handle<Object> self = args.Holder();  
  
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));  
    void* ptr = wrap->Value();  
    CloudApp* cloudapp = static_cast<CloudApp*>(ptr);  
  
    cloudapp->start();  
  
    return Undefined();  
}  
  
void SetupCloudAppInterface(Handle<ObjectTemplate> global) 
{  
    Handle<FunctionTemplate> cloudapp_class = FunctionTemplate::New(CloudAppConstructCallback);  
    cloudapp_class->SetClassName(String::New("CloudApp"));  
	cloudapp_class->Set(String::New("CreateApp"), FunctionTemplate::New(JS_CreateApp));
  
    Handle<ObjectTemplate> cloudapp_proto = cloudapp_class->PrototypeTemplate();  
    //��һ������ȫ����ʹ��cloudapp_inst->Set��....��  
    //ʹ��prototype������JS���  
    cloudapp_proto->Set(String::New("start"),	FunctionTemplate::New(JS_Start));  
    cloudapp_proto->Set(String::New("state"),	FunctionTemplate::New(JS_GetState));  
    cloudapp_proto->Set(String::New("appid"),	FunctionTemplate::New(JS_GetAppId));  
	cloudapp_proto->Set(String::New("IsEqual"), FunctionTemplate::New(JS_IsEqual));
    //******����Ҫ������  
    Handle<ObjectTemplate> cloudapp_inst = cloudapp_class->InstanceTemplate();  
    cloudapp_inst->SetInternalFieldCount(1);  
      
    //��JS����ע��һ���������䱾�ʾ�����JS�����globalע��һ���ࡣ  
    //���ԣ�Ҳ��ͨ����globalע��CloudApp�ࡣ  
    global->Set(String::New("CloudApp"), cloudapp_class);  
}  
  
void InitialnilizeInterface(Handle<ObjectTemplate> global)
{  
    SetupCloudAppInterface(global);  
}  

char *jsscritp =  "\
var ca = CloudApp.CreateApp();\
var caid = ca.getAppId();\
var app = new CloudApp( 10 ); \
var app2 = new CloudApp( 20); \
var app3 = new CloudApp( 30);\
var apparray = [app2, app3];\
var arrint = [1, 2 ];\
var bo = app.IsEqual(apparray);\
{\
	var te = new CloudApp(9); \
	te = null; \
}\
var id = app.appid(); \
app.start();\
app = null"	\
;
  
void LoadJsAndRun()
{  
	Handle<String> source = String::New( jsscritp );  
    Handle<Script> script = Script::Compile(source);  
    Handle<Value> result = script->Run();  
  
    //printValue(result);  
}  
  
void Regist2JsContext(Handle<ObjectTemplate>& object  
                            , Persistent<Context>& context) {  
    context = Context::New(NULL, object);  
}  
 

Handle<Value> Yell(const Arguments& args) {  
	HandleScope  handle_scope;  
	char buffer[4096];  

	memset(buffer, 0, sizeof(buffer));  
	Handle<String> str = args[0]->ToString();  
	str->WriteAscii(buffer);  
	printf("Yell: %s\n", buffer);  

	return Undefined();  
}  

int main2(int argc, char** argv) 
{  
	HandleScope handle_scope;  

	//A  
	Handle<FunctionTemplate> fun = FunctionTemplate::New(Yell);  

	//B  
	Handle<ObjectTemplate> global = ObjectTemplate::New();  
	global->Set(String::New("yell"), fun);  

	//C  
	Persistent<Context> cxt = Context::New(NULL, global);  

	Context::Scope context_scope(cxt);  
	Handle<String> source = String::New("yell('Google V8!')");  
	Handle<Script> script = Script::Compile(source);  
	Handle<Value> result = script->Run();  

	cxt.Dispose(); 
	return 0;
}  


int main(int argc, char** argv)
{  
    HandleScope handle_scope;  
    Handle<ObjectTemplate> global = ObjectTemplate::New();  
    Persistent<Context> context;  
      
    InitialnilizeInterface(global);  
    Regist2JsContext(global, context);  
    Context::Scope context_scope(context);  
    LoadJsAndRun();  
  
    context.Dispose();  
	
	V8::LowMemoryNotification();
      
    return 0;  
}  