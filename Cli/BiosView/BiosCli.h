#include <memory>
#include "DataModel.h"
#include "Volume.h"

#ifdef _WIN32
#include "curses.h"
#else
#include <ncurses.h>
#endif
#include "IfwiRegion/BiosRegion.h"

using std::unique_ptr;

class BiosCliData {
public:
    vector<Volume *> VolumeDataList{};
    Volume *OverviewVolume{nullptr};
    BiosRegion *BiosImage{nullptr};
    DataModel *OverviewImageModel{nullptr};

    bool BiosValidFlag{true};
    bool IFWI_exist{false};
    bool infoWindowOpened{false};
    bool searchDialogOpened{false};

    BiosCliData() = default;
    ~BiosCliData();
    static bool isValidBIOS(UINT8 *image, INT64 imageLength);
};

class TreeNode {
public:
    DataModel data;
    TreeNode *parent = nullptr;
    vector<unique_ptr<TreeNode>> children;
    bool isExpanded = false;
    int depth = 0;

    TreeNode(DataModel dm, TreeNode *p = nullptr, int d = 0) : data(std::move(dm)), parent(p), depth(d) {}
};

class BiosCliView {
private:
    unique_ptr<TreeNode> dataRoot; // 树形数据结构根节点
    vector<TreeNode *> visibleNodes; // 当前可见节点列表
    WINDOW *left_win;
    WINDOW *right_win;
    int leftTableWidth = 0;
    int rightPanelWidth = 0;
    int colWidths[3] = {36, 10, 15};
    void buildTree(Volume *volume, TreeNode *parent, int depth);
    void rebuildVisibleList();
    void drawTable(int startRow);
    void drawPanel();

public:
    // Data
    BinaryData *binaryData{nullptr};
    BiosCliData *BiosData{nullptr};
    vector<DataModel> tableData;
    int selectedRow = 0; // 当前选中行
    int scrollOffset = 0; // 滚动偏移量

    explicit BiosCliView(BinaryData *binary);
    ~BiosCliView();
    void loadBios();

    bool detectIfwi(INT64 &BiosOffset) const;
    void setBiosFvData();
    void DecodeBiosFileSystem();
    void AddVolumeList(INT64 offset, INT64 length, Volume *parent, VolumeType type) const;
    void setTreeData();
    void ReorganizeVolume(Volume *volume);
    // void addTreeItem(QTreeWidgetItem *parentItem, Volume *volume, bool ShowPadding);
};
