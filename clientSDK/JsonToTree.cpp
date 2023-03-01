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
	void addpara_double(double a) {
		para_double.push_back(a);
	}
	void addpara_int(int a) {
		para_int.push_back(a);
	}
	void addpara_string(string a) {
		para_string.push_back(a);
	}
	void addpara_mat(cv::Mat a) {
		para_mat.push_back(a);
	}
	virtual void Run() {
		bool cando = true;
		for (auto parent : m_parents) {
			if (resMap.find(parent->getid()) == resMap.end())
			{
				cando = false;
				break;
			}
		}//�����Ѿ��ж����丸�ڵ�Ľ��������
		if (cando) {
			cv::Mat mat;
			std::cout << "Running node " << m_name << std::endl;
			std::cout << "dllpath " << dllpath << std::endl;
			std::cout << "dllname " << dllname << std::endl;
			//�����ͼ������
			if (m_name == "���ļ�") {
				std::cout << "���ļ�" << dllname << std::endl;
			}
			//if (m_name == "ͼ���˲�") {
			{   //���ﱾ�õ���ͼ���˲�����������û�ı����ʽ�����Ľ����Ȼ�����룬�����жϲ����������ǿ�����ȷ�õ�xy��ֵ
				//������������˲������������������������ͺͽ�㴫��˳����ڸ�����Ӧ�ı����棬����sigmaX����para_Double[0],Y��para_Double[1];
				//�������ֻʵ���˼������õ��������ͣ���������ÿ������м���protected����
				//ȡ����Ӧ������������������dll,dll�����ƺ�·����Ҳ������dllpath��dllname�У������е���
				//����ĺ���ͼ����ñ���Ӧ��ʹ��ö�٣�switch��д�������Ҳ���������ж���ͼ�����㣬������ʱ�õ�if��֮����������Ż�
				std::cout << "ͼ���˲�" << dllname << std::endl;
				std::cout << "ksize" << para_int[0] << std::endl;
				std::cout << "borderType" << para_int[1] << std::endl;
				std::cout << "borderType" << para_mat.size() << std::endl;

				std::cout << "sigmaX" << para_double[0] << std::endl;
				std::cout << "sigmaY" << para_double[1] << std::endl;

			}
			if (m_name == "imgFilter") {
				// ����ʱ����DLL��
				const char* path = (dllpath + dllname + ".dll").data();
				HMODULE module = LoadLibrary(path);     // ����DLL�ļ���������DLL������һ��ģ����
				if (module == NULL)
				{
					printf("����dll��̬��ʧ��\n");
					return;
				}
				if (dllname == "GaussianBlur") {
					typedef void(*Func)(InputArray, OutputArray, Size, double, double, int);                  // ���庯��ָ������
					// ����������ַ
					Func f = (Func)GetProcAddress(module, "myGaussianBlur");
					if (f == NULL)
					{
						printf("���ض�̬���еķ���ʧ��\n");
						return;
					}
					printf("���غ����ɹ�");
					
				}
			}

			resMap[this->m_id] = mat;//��ִ�н�������ϣ������������ݺ��������Լ�д��������ж�����
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
	std::vector <int> para_int;//�����int����
	std::vector <double> para_double;
	std::vector <std::string> para_string;
	std::vector<cv::Mat> para_mat;
		//�����Ҫ����Ĳ������ͣ����������������
};

class LoopNode : public TreeNode {
public:
	LoopNode(const std::string& name, int id, const std::string& dlname, const std::string& dlpath, const std::string& condition)
		: TreeNode(name, id, dlname, dlpath), m_condition(condition) {}

	virtual void Run() override {
		//����Ҫ�ж�����ڵ�ĸ��ڵ��ͼ������
		//��resMap[m_parents[i]->id]!=null;
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
		// �ж�
		return m_id < 5;
	}

	std::string m_condition;
};

class ConditionalNode : public TreeNode {
public:
	ConditionalNode(const std::string& name, int id, const std::string& dlname, const std::string& dlpath, const std::string& condition)
		: TreeNode(name, id, dlname, dlpath), m_condition(condition) {}

	virtual void Run() override {
		//����Ҫ�ж�����ڵ�ĸ��ڵ��ͼ������
		//��resMap[m_parents[i]->id]!=null;
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
		//�ж�
		return m_condition == "True";
	}

	std::string m_condition;
};
std::unordered_map<int, cv::Mat> TreeNode::resMap;
std::unordered_map<int, std::vector<int>> TreeNode::int_Map;
std::unordered_map<int, std::vector<double>> TreeNode::double_Map;
std::unordered_map<int, std::vector<std::string>> TreeNode::string_Map;
TreeNode* CreateNode(const json& nodeJson, std::unordered_map<int, TreeNode*>& nodeMap) {
	json properties = nodeJson["properties"].get<json>();
	json inPara = properties["inPara"].get<json>();
	int id = properties["id"].get<int>();

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
		if (para["varType"].get<std::string>() == "double")
			node->addpara_double(strtod(para["value"].get<std::string>().c_str(), NULL));

		else if (para["varType"].get<std::string>() == "int")
			node->addpara_int(stoi(para["value"].get<std::string>()));

		else if (para["varType"].get<std::string>() == "string") 
			node->addpara_string(para["value"].get<std::string>());
	
		else if (para["varType"].get<std::string>() == "Mat") {
			//���������������ô�������base64��
			//��������ã��ǾͿ�ֵ��������һ���Ľ��
			//�����ָ�룬Ҳ�ǿ�ֵ��������֮��ý������
			cv::Mat m;
			if (para["valueType"].get<int>() == 0) {
				//����
				string base64 = para["value"].get<std::string>();
				m = Base2Mat(base64);
			}
			node->addpara_mat(m);
		}
	}
	nodeMap[id] = node;
	return node;
}

void LinkNodes(const json& edgeJson, std::unordered_map<int, TreeNode*>& nodeMap) {
	int childId = edgeJson["to"].get<int>();
	int parentId = edgeJson["from"].get<int>();
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
	//ʵ��
}

int main(int argc, char** argv) {
	/*if (argc < 2) {
		std::cout << "Usage: ./program <path_to_json_file>" << std::endl;
		return 1;
	}*/
	std::string jsonPath = "example.json";
	std::ifstream jsonFile(jsonPath);
	if (!jsonFile.is_open()) {
		std::cout << "Failed to open file: " << jsonPath << std::endl;
		return 1;
	}

	json jsonInput;
	jsonFile >> jsonInput;
	jsonFile.close();

	std::unordered_map<int, TreeNode*> nodeMap;

	BuildTree(jsonInput, nodeMap);

	return 0;
}