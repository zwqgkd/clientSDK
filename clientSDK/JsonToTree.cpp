#include "JsonToTree.h"
#include "base64.h"
#include<Windows.h>

using json = nlohmann::json;
using namespace std;
using namespace cv;
class TreeNode {
public:
	static std::unordered_map<int, cv::Mat> resMap;
	static std::unordered_map<int, std::vector<int>> int_Map;
	static std::unordered_map<int, std::vector<double>> double_Map;
	static std::unordered_map<int, std::vector<std::string>> string_Map;

	static JNIEnv *env;
	static jobject listener;
	static jmethodID method;

	int getid() {
		return m_id;
	}
	TreeNode(const std::string& name, int id, const std::string& dlname, const std::string& dlpath)
		: m_name(name), m_id(id), dllpath(dlpath), dllname(dlname) {}

	virtual ~TreeNode() {}

	void AddChild(TreeNode* child) {
		m_children.push_back(child);
	}

	void AddParent(TreeNode* parent) {
		m_parents.push_back(parent);
	}
	void addpara(string a) {
		para_string.push_back(a);
	}

	virtual void Run() {
		bool cando = true;
		for (auto parent : m_parents) {
			if (resMap.find(parent->getid()) == resMap.end())
			{
				cando = false;
				break;
			}
		}//这里已经判断了其父节点的结果都存在

		if (cando) {
			cv::Mat mat;
			std::cout << "Running node " << m_name << std::endl;
			cout << "the node path ：" << dllpath + dllname + ".dll" << endl;
			//这里放图像处理函数

			//if (m_name == "图像滤波") {
			{   //这里本该调用图像滤波，但是由于没改编码格式，中文结点依然是乱码，所以判断不出来，但是可以正确得到xy的值
				//在这里面调用滤波函数，函数参数按数据类型和结点传入顺序存在该类相应的表里面，列如sigmaX会是para_Double[0],Y是para_Double[1];
				//这个表里只实现了几个常用的数据类型，如果不够用可以自行加在protected底下
				//取出相应参数后可以在下面调用dll,dll的名称和路径我也存在了dllpath和dllname中，请自行调用
				//这里的函数图像调用本来应该使用枚举＋switch来写，但是我不清楚具体有多少图像处理结点，所以暂时用的if，之后可以自行优化

			}
			// 运行时加载DLL库
			string p = "";
			p.append(dllpath);
			p.append(dllname);
			p.append(".dll");
			const char* path = p.data();

			env->CallVoidMethod(listener, method, stoJstring(env, "before load dll"));

			if (m_name == "imgFilter") {
				HMODULE module = LoadLibrary("C:/Projects/cpp/clientSDK/clientSDK/dll/GaussianBlur.dll");     // 根据DLL文件名，加载DLL，返回一个模块句柄
				if (module == NULL)
				{
					cout << "load dll failed" << endl;
					return;
				}
				// 定义函数指针类型
				typedef void(*Func)(InputArray, OutputArray, Size, double, double, int);                  
				// 导出函数地址
				Func f = (Func)GetProcAddress(module, dllname.data());
				if (f == NULL)
				{
					cout << "load method failed" << endl;
					return;
				}
				//调用函数
				cv::Mat src = resMap[GetParents()[0]->getid()];
				cv::Mat dst;
				Size ksize(5, 5);
				double sigmax = stod(para_string[3]);
				double sigmay = stod(para_string[4]);
				int borderType = stoi(para_string[5]);
				f(src, dst, ksize, sigmax, sigmay, borderType);
				//dst作为结果存到resMap
				resMap[this->m_id] = dst;
				//test
				env->CallVoidMethod(listener, method, stoJstring(env, "imgFilter done"));
			}
			else if (m_name == "inputFile") {
				HMODULE module = LoadLibrary("C:/Projects/cpp/clientSDK/clientSDK/dll/Read.dll");     // 根据DLL文件名，加载DLL，返回一个模块句柄
				if (module == NULL)
				{
					cout << "load dll failed" << endl;
					return;
				}
				// 定义函数指针类型
				typedef cv::Mat(*Func)(cv::String, int );
				// 导出函数地址
				Func f = (Func)GetProcAddress(module, dllname.data());
				if (f == NULL)
				{
					cout << "load method failed" << endl;
					return;
				}
				//调用函数
				cv::String filename = para_string[0];
				int flags = stoi(para_string[1]);
				
				//结果存到resMap
				resMap[this->m_id] = f(filename,flags);
				env->CallVoidMethod(listener, method, stoJstring(env, "inputFile done"));
			}
			//resMap[this->m_id] = mat;//将执行结果存入哈希表，其他结果根据函数类型自己写在上面的判断里面
			for (auto child : m_children) {
				child->Run();
			}
		}
	}
	std::vector<TreeNode*> GetParents() {
		return this->m_parents;
	}
protected:
	std::string m_name;
	int m_id;
	std::vector<TreeNode*> m_children;
	std::vector<TreeNode*> m_parents;
	std::string dllpath;
	std::string dllname;
	std::vector <std::string> para_string;
};

class LoopNode : public TreeNode {
public:
	LoopNode(const std::string& name, int id, const std::string& dlname, const std::string& dlpath, const std::string& condition)
		: TreeNode(name, id, dlname, dlpath), m_condition(condition) {}

	virtual void Run() override {
		//这里要判断这个节点的父节点的图都存在
		//即resMap[m_parents[i]->id]!=null;
		std::cout << "Running loop node " << m_name << std::endl;
		cv::Mat res;
		resMap[this->m_id] = res;
		while (CheckCondition()) {
			for (auto child : m_children) {
				child->Run();
			}
		}
	}

private:
	bool CheckCondition() {
		// 判断
		return m_id < 5;
	}

	std::string m_condition;
};

class ConditionalNode : public TreeNode {
public:
	ConditionalNode(const std::string& name, int id, const std::string& dlname, const std::string& dlpath, const std::string& condition)
		: TreeNode(name, id, dlname, dlpath), m_condition(condition) {}

	virtual void Run() override {
		//这里要判断这个节点的父节点的图都存在
		//即resMap[m_parents[i]->id]!=null;
		std::cout << "Running conditional node " << m_name << std::endl;
		cv::Mat res;
		resMap[this->m_id] = res;
		if (CheckCondition()) {
			if (!m_children.empty()) {
				m_children[0]->Run();
			}
		}
		else {
			if (m_children.size() > 1) {
				m_children[1]->Run();
			}
		}
	}

private:
	bool CheckCondition() {
		//判断
		return m_condition == "True";
	}

	std::string m_condition;
};
std::unordered_map<int, cv::Mat> TreeNode::resMap;
std::unordered_map<int, std::vector<int>> TreeNode::int_Map;
std::unordered_map<int, std::vector<double>> TreeNode::double_Map;
std::unordered_map<int, std::vector<std::string>> TreeNode::string_Map;

JNIEnv *TreeNode::env;
jobject TreeNode::listener;
jmethodID TreeNode::method;

TreeNode* CreateNode(const json& nodeJson, std::unordered_map<int, TreeNode*>& nodeMap) {
	json properties = nodeJson["properties"].get<json>();
	json inPara = properties["inPara"].get<json>();
	int id = stoi(nodeJson["id"].get<string>());

	std::string type = nodeJson["type"].get<std::string>();
	std::string dllpath = properties["dllPath"].get<std::string>();
	std::string dllname = properties["methodName"].get<std::string>();
	std::string name = properties["name"].get<std::string>();

	TreeNode* node = nullptr;
	if (type == "loop") {
		std::string condition = nodeJson["condition"].get<std::string>();
		node = new LoopNode(name, id, dllname, dllpath, condition);
	}
	else if (type == "condition") {
		std::string condition = nodeJson["condition"].get<std::string>();
		node = new ConditionalNode(name, id, dllname, dllpath, condition);
	}
	else {
		node = new TreeNode(name, id, dllname, dllpath);

	}

	for (const auto& para : inPara) {
		node->addpara(para["value"].get<std::string>());
	}
	nodeMap[id] = node;
	return node;
}

void LinkNodes(const json& edgeJson, std::unordered_map<int, TreeNode*>& nodeMap) {
	int childId = stoi(edgeJson["targetNodeId"].get<string>());
	int parentId = stoi(edgeJson["sourceNodeId"].get<string>());
	TreeNode* child = nodeMap[childId];
	TreeNode* parent = nodeMap[parentId];
	child->AddParent(parent);
	parent->AddChild(child);
}
void BuildTree(const json& jsonInput, std::unordered_map<int, TreeNode*>& nodeMap) {
	// Create nodes
	for (const auto& nodeJson : jsonInput["nodes"]) {
		CreateNode(nodeJson, nodeMap);
	}// Link nodes
	for (const auto& edgeJson : jsonInput["edges"]) {
		LinkNodes(edgeJson, nodeMap);
	}

	// Find root node
	TreeNode* rootNode = nullptr;
	for (auto it : nodeMap) {
		if (it.second->GetParents().empty()) {
			if (rootNode == nullptr) {
				rootNode = it.second;
			}
			else {
				throw std::runtime_error("Multiple root nodes found");
			}
		}
	}

	// Run root node
	if (rootNode != nullptr) {
		rootNode->Run();
	}
	else {
		throw std::runtime_error("No root node found");
	}
}



void jsonHandle(string paraStr, JNIEnv *env, jobject listener, jmethodID method) {
	//
	TreeNode::env = env;
	TreeNode::listener = listener;
	TreeNode::method = method;
	std::unordered_map<int, TreeNode*> nodeMap;
	BuildTree(json::parse(paraStr), nodeMap);
	string res = "success";
	env->CallVoidMethod(listener, method, stoJstring(env, res.data()));
}

////测试
//int main(int argc, char** argv) {
//	/*if (argc < 2) {
//		std::cout << "Usage: ./program <path_to_json_file>" << std::endl;
//		return 1;
//	}*/
//	std::string jsonPath = "example.json";
//	std::ifstream jsonFile(jsonPath);
//	if (!jsonFile.is_open()) {
//		std::cout << "Failed to open file: " << jsonPath << std::endl;
//		return 1;
//	}
//
//	json jsonInput;
//	jsonFile >> jsonInput;
//	jsonFile.close();
//
//	std::unordered_map<int, TreeNode*> nodeMap;
//
//	BuildTree(jsonInput, nodeMap);
//
//	return 0;
//}