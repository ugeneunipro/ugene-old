#include <QMap>
#include <QString>
#include <U2Algorithm/SubstMatrixRegistry.h>
#include <U2Core/MAlignment.h>
#include <U2Core/PhyTree.h>
#include <U2Algorithm/CreatePhyTreeSettings.h>
#include <QVector>
#include <iostream>

namespace U2{

typedef QVector<float> matrixrow;
typedef QVector<matrixrow> matrix;

class DistanceMatrix{
private:
	float* rawdata;
	int size;
	QMap<QString, int> index_map;
	const MAlignment* malignment;
	PhyTreeData* treedata;
	matrix qdata;
	matrix middlematrix;
	QList<QString> visited_list;
	QList<QString> unprocessed_taxa;
	QList<PhyNode*> printed_nodes;

public:
    matrix rawMatrix;
    bool isValid();
    void calculateOutOfAlignment(const MAlignment& ma, const CreatePhyTreeSettings& settings);
    ~DistanceMatrix();

private:
    
    //TODO: this stuff is obsolete. Consider deleting this.
    void  initializePhyTree();
    void  calculateQMatrix();
	float calculateRawDivergence (int i);
	float getDistance (QString seq1, QString seq2);
	void  dumpRawData(matrix& m);
	void  dumpQData();
	QPair<int, int> *getLowestLocation();
	QString getTaxaName(int index);
	void addPairToTree(QPair<int, int> *location);
	int getSize(){return size;}
	PhyNode* getNodeByName(QString name);
	PhyNode* findNode(PhyNode* startnode, QString name);
	float calculateRootDistance(int i, int j);
	float calculateAdjacentDistance(int i, int j, float rootDistance);
	QString generateNodeName(QString name1, QString name2);
	void recalculateDistanceMatrix(int i, int j, PhyNode* newnode);
	float calculateNewDistance(QPair<int, int>* location, int current);
	bool hasUnprocessedTaxa(){
		return !unprocessed_taxa.empty();
	}
	static void printPhyTree(PhyTreeData* treeData);
	void printPhyTree();
	static void printPhyNode(PhyNode* node, int tab, QList<PhyNode*>& nodes);
	void printIndex();
	PhyNode* findParent(PhyNode* node);
	int getNewIndex(QString name, QPair<int, int> loc, QMap<QString, int>& old_map);
	void dumpRawData();
	PhyTreeData* getTree(){return treedata;}
	static bool areTreesEqual(PhyTree* tree1, PhyTree* tree2);
	static void addNodeToList(QList<PhyNode*>& nodelist, QMap<QString, int>& nodemap, QList<PhyBranch*>& branches,  PhyNode* node);
	void switchName(PhyNode* node);
	void switchNamesToAllNodes();
};



}
