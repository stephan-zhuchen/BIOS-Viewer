#include "BiosCli.h"
#include <filesystem>
#include <functional>
#include <iostream>
#include <thread>
#include "BaseLib.h"
#include "IfwiRegion/BiosRegion.h"
#include "IfwiRegion/EcRegion.h"
#include "IfwiRegion/FlashDescriptorRegion.h"
#include "IfwiRegion/GbeRegion.h"
#include "IfwiRegion/MeRegion.h"
#include "IfwiRegion/OsseRegion.h"
#include "UEFI/GuidDatabase.h"
#include "UefiFileSystem/CompressedVolume.h"
#include "UefiFileSystem/FirmwareVolume.h"

using namespace BaseLibrarySpace;
GuidDatabase *guidData = nullptr;

BiosCliView::BiosCliView(BinaryData *binary) : binaryData(binary) {
    if (guidData == nullptr) {
        guidData = new GuidDatabase;
    }
    leftTableWidth = colWidths[0] + colWidths[1] + colWidths[2] + 6;
}

BiosCliView::~BiosCliView() {
    delete guidData;
    delete BiosData;
}

void BiosCliView::loadBios() {
    BiosData = new BiosCliData;
    BiosData->OverviewVolume = new Volume(binaryData->InputImage, binaryData->InputImageSize);
    BiosData->OverviewImageModel = new DataModel(BiosData->OverviewVolume, "IFWI Overview", "Image");

    // 创建虚拟根节点
    dataRoot = make_unique<TreeNode>(DataModel(nullptr, "ROOT", "Virtual Root"), nullptr, -1);

    setBiosFvData();
    DecodeBiosFileSystem();
    for (auto fv : BiosData->VolumeDataList) {
        ReorganizeVolume(fv);
        buildTree(fv, dataRoot.get(), 0); // 将每个FV挂载到虚拟根节点下
    }
    if (BiosData->BiosValidFlag && BiosData->BiosImage->isFitValid()) {
        if (!BiosData->IFWI_exist) {
            for (auto vol : BiosData->VolumeDataList) {
                BiosData->BiosImage->ChildVolume.push_back(vol);
            }
        }
        BiosData->BiosImage->setBiosID();
        BiosData->BiosImage->setInfoStr();
        BiosData->OverviewVolume->setInfoText(BiosData->BiosImage->getInfoText());
    }

    string title;
    if (BiosData->BiosImage->getBiosID() == "") {
        filesystem::path filePath(binaryData->OpenedFileName);
        title = filePath.filename().string();
    } else
        title = BiosData->BiosImage->getBiosID();

    cout << title << endl;

    // 初始化界面
    rebuildVisibleList();
    setTreeData();
}

bool BiosCliData::isValidBIOS(UINT8 *image, INT64 imageLength) {
    if (imageLength == 0x2000000) {
        FlashDescriptorRegion flashDescriptor = FlashDescriptorRegion(image, imageLength, 0);
        if (flashDescriptor.CheckValidation()) {
            return true;
        }
    }

    INT64 SearchOffset = 0;
    INT64 SearchInterval = 0x40;
    while (SearchOffset <= imageLength - SearchInterval) {
        FirmwareVolume fvHeader = FirmwareVolume(image + SearchOffset, imageLength - SearchOffset, SearchOffset);
        if (fvHeader.CheckValidation()) {
            return true;
        }
        SearchOffset += SearchInterval;
    }

    return false;
}

bool BiosCliView::detectIfwi(INT64 &BiosOffset) const {
    using namespace std;

    INT64 bufferSize = binaryData->InputImageSize;
    if (bufferSize < 0x4000) {
        return false;
    }

    auto CleanVolumeDataList = [this]() {
        for (Volume *vol : BiosData->VolumeDataList) {
            safeDelete(vol);
        }
        BiosData->VolumeDataList.clear();
    };

    INT64 IfwiOffset = 0;
    auto *flashDescriptor = new FlashDescriptorRegion(binaryData->InputImage, bufferSize, IfwiOffset);
    if (flashDescriptor->SelfDecode() == 0) {
        safeDelete(flashDescriptor);
        CleanVolumeDataList();
        return false;
    }
    BiosData->VolumeDataList.push_back(flashDescriptor);

    FlashRegionBaseArea BiosRegionArea = flashDescriptor->RegionList.at(FLASH_REGION_TYPE::FlashRegionBios);
    FlashRegionBaseArea MeRegionArea = flashDescriptor->RegionList.at(FLASH_REGION_TYPE::FlashRegionMe);
    FlashRegionBaseArea GbERegionArea = flashDescriptor->RegionList.at(FLASH_REGION_TYPE::FlashRegionGbE);
    FlashRegionBaseArea EcRegionArea = flashDescriptor->RegionList.at(FLASH_REGION_TYPE::FlashRegionEC);
    FlashRegionBaseArea OsseRegionArea = flashDescriptor->RegionList.at(FLASH_REGION_TYPE::FlashRegionIE);

    if (EcRegionArea.getLimit() > bufferSize) {
        CleanVolumeDataList();
        return false;
    }

    if (EcRegionArea.limit != 0) {
        UINT8 *EcBuffer = binaryData->InputImage + EcRegionArea.getBase();
        auto *EcVolume = new EcRegion(EcBuffer, EcRegionArea.getSize(), EcRegionArea.getBase());
        if (EcVolume->SelfDecode() == 0) {
            safeDelete(EcVolume);
            CleanVolumeDataList();
            return false;
        }
        BiosData->VolumeDataList.push_back(EcVolume);
    }

    if (GbERegionArea.getLimit() > bufferSize) {
        CleanVolumeDataList();
        return false;
    }
    if (GbERegionArea.limit != 0) {
        UINT8 *GbeBuffer = binaryData->InputImage + GbERegionArea.getBase();
        auto *GbEVolume = new GbeRegion(GbeBuffer, GbERegionArea.getSize(), GbERegionArea.getBase());
        if (GbEVolume->SelfDecode() == 0) {
            safeDelete(GbEVolume);
            CleanVolumeDataList();
            return false;
        }
        BiosData->VolumeDataList.push_back(GbEVolume);
    }

    if (MeRegionArea.getLimit() > bufferSize) {
        CleanVolumeDataList();
        return false;
    }
    if (MeRegionArea.limit != 0) {
        UINT8 *MeBuffer = binaryData->InputImage + MeRegionArea.getBase();
        auto *MeVolume = new MeRegion(MeBuffer, MeRegionArea.getSize(), MeRegionArea.getBase());
        if (MeVolume->SelfDecode() == 0) {
            safeDelete(MeVolume);
            CleanVolumeDataList();
            return false;
        }
        BiosData->VolumeDataList.push_back(MeVolume);
    }

    if (OsseRegionArea.getLimit() > bufferSize) {
        CleanVolumeDataList();
        return false;
    }
    if (OsseRegionArea.limit != 0) {
        UINT8 *GbeBuffer = binaryData->InputImage + OsseRegionArea.getBase();
        auto *OsseVolume = new OsseRegion(GbeBuffer, OsseRegionArea.getSize(), OsseRegionArea.getBase());
        if (OsseVolume->SelfDecode() == 0) {
            safeDelete(OsseVolume);
            CleanVolumeDataList();
            return false;
        }
        BiosData->VolumeDataList.push_back(OsseVolume);
    }

    if (BiosRegionArea.getLimit() > bufferSize) {
        CleanVolumeDataList();
        return false;
    }
    if (BiosRegionArea.limit != 0) {
        UINT8 *BiosBuffer = binaryData->InputImage + BiosRegionArea.getBase();
        BiosData->BiosImage = new BiosRegion(BiosBuffer, BiosRegionArea.getSize(), BiosRegionArea.getBase());
        BiosData->VolumeDataList.push_back(BiosData->BiosImage);
        BiosOffset = BiosRegionArea.getBase();
    }
    return true;
}

void BiosCliView::setBiosFvData() {
    using namespace std;
    INT64 offset = 0;
    INT64 bufferSize = binaryData->InputImageSize;
    BiosData->IFWI_exist = detectIfwi(offset);

    Volume *parentVolume = BiosData->BiosImage;
    if (!BiosData->IFWI_exist) {
        parentVolume = nullptr;
        BiosData->BiosImage = new BiosRegion(binaryData->InputImage, bufferSize);
        BiosData->OverviewImageModel->setName("BIOS Image Overview");
    }
    BiosData->BiosImage->SelfDecode();

    while (offset < bufferSize) {
        if (bufferSize - offset < 0x40) {
            AddVolumeList(offset, bufferSize - offset, parentVolume, VolumeType::Empty);
            return;
        }
        auto fvHeader = (EFI_FIRMWARE_VOLUME_HEADER *) (binaryData->InputImage + offset);
        INT64 FvLength = (INT64) fvHeader->FvLength;
        bool IsFirmwareVolume = FirmwareVolume::isValidFirmwareVolume(fvHeader);

        auto CompressedVolumeHeader = (LOADER_COMPRESSED_HEADER *) (binaryData->InputImage + offset);
        INT64 CompressedVolumeLength = sizeof(LOADER_COMPRESSED_HEADER) + CompressedVolumeHeader->CompressedSize;
        bool IsCompressedVolume = CompressedVolume::IsCompressedVolume(CompressedVolumeHeader);

        INT64 searchInterval = 0x40;
        INT64 EmptyVolumeLength = 0;
        while (!IsFirmwareVolume && !IsCompressedVolume) {
            EmptyVolumeLength += searchInterval;
            if (offset + EmptyVolumeLength >= bufferSize) {
                AddVolumeList(offset, bufferSize - offset, parentVolume, VolumeType::Empty);
                return;
            }
            fvHeader = (EFI_FIRMWARE_VOLUME_HEADER *) (binaryData->InputImage + offset + EmptyVolumeLength);
            CompressedVolumeHeader = (LOADER_COMPRESSED_HEADER *) (binaryData->InputImage + offset + EmptyVolumeLength);
            IsFirmwareVolume = FirmwareVolume::isValidFirmwareVolume(fvHeader);
            IsCompressedVolume = CompressedVolume::IsCompressedVolume(CompressedVolumeHeader);
        }

        if (offset + EmptyVolumeLength == bufferSize && offset == 0) {
            return;
        }

        if (EmptyVolumeLength != 0) {
            AddVolumeList(offset, EmptyVolumeLength, parentVolume, VolumeType::Empty);
            offset += EmptyVolumeLength;
            continue;
        }

        if (IsFirmwareVolume) {
            AddVolumeList(offset, FvLength, parentVolume, VolumeType::FirmwareVolume);
            offset += FvLength;
        } else if (IsCompressedVolume) {
            AddVolumeList(offset, CompressedVolumeLength, parentVolume, VolumeType::Compressed);
            offset += CompressedVolumeLength;
            Align(offset, 0, 0x1000);
        }
    }
}

void BiosCliView::DecodeBiosFileSystem() {
    using namespace std;
    if (BiosData->VolumeDataList.size() == 1 && BiosData->VolumeDataList.at(0)->getVolumeType() == VolumeType::Empty) {
        delete BiosData->VolumeDataList.at(0);
        BiosData->VolumeDataList.clear();
        BiosData->BiosValidFlag = false;
        return;
    }

    vector<thread> threadPool;
    auto FvDecoder = [this](size_t index) {
        Volume *volume = BiosData->VolumeDataList.at(index);
        volume->DecodeChildVolume();
    };
    for (size_t idx = 0; idx < BiosData->VolumeDataList.size(); ++idx) {
        threadPool.emplace_back(FvDecoder, idx);
    }
    for (thread &t : threadPool) {
        t.join();
    }
}

void BiosCliView::AddVolumeList(INT64 offset, INT64 length, Volume *parent, VolumeType type) const {
    Volume *volume{nullptr};
    UINT8 *volumeData = binaryData->InputImage + offset;
    if (type == VolumeType::Empty) {
        volume = new Volume(volumeData, length, offset, false, parent);
    } else if (type == VolumeType::FirmwareVolume) {
        try {
            volume = new FirmwareVolume(volumeData, length, offset, false, parent);
            if (volume->SelfDecode() == 0) {
                safeDelete(volume);
                volume = new Volume(volumeData, length, offset, false, parent);
            }
        } catch (...) {
            safeDelete(volume);
            volume = new Volume(volumeData, length, offset, false, parent);
        }
    } else if (type == VolumeType::Compressed) {
        try {
            volume = new CompressedVolume(volumeData, length, offset, parent);
            if (volume->SelfDecode() == 0) {
                safeDelete(volume);
                volume = new Volume(volumeData, length, offset, false, parent);
            }
        } catch (...) {
            safeDelete(volume);
            volume = new Volume(volumeData, length, offset, false, parent);
        }
    }

    if (parent == nullptr) {
        BiosData->VolumeDataList.push_back(volume);
    } else {
        parent->ChildVolume.push_back(volume);
    }
}

// 树形结构构建函数
void BiosCliView::buildTree(Volume *volume, TreeNode *parent, UINT64 depth) {
    // 创建数据模型
    DataModel model;
    model.InitFromVolume(volume);
    model.setName(string(depth, ' ') + model.getName());

    // 创建树节点
    auto newNode = make_unique<TreeNode>(model, parent, (int)depth);
    TreeNode *rawNode = newNode.get();
    rawNode->isExpanded = false;

    // 递归构建子树
    for (auto child : volume->ChildVolume) {
        buildTree(child, rawNode, depth + 1);
    }

    // 挂载到父节点
    if (parent) {
        parent->children.push_back(std::move(newNode));
    }
}

// 可见列表重建函数
void BiosCliView::rebuildVisibleList() {
    visibleNodes.clear();

    function<void(TreeNode *)> depthFirstTraversal = [&](TreeNode *node) {
        visibleNodes.push_back(node);
        if (node->isExpanded) {
            for (auto &child : node->children) {
                depthFirstTraversal(child.get());
            }
        }
    };

    // 从每个根子节点开始遍历
    for (auto &child : dataRoot->children) {
        depthFirstTraversal(child.get());
    }
}

// 绘制表格框架
void BiosCliView::drawTable(UINT64 startRow) {
    // clear();
    werase(left_win);

    // 表头
    wattron(left_win, A_BOLD);
    mvwprintw(left_win,
              0,
              0,
              "%-*s | %-*s | %-*s",
              colWidths[0],
              "Volume Name",
              colWidths[1],
              "Type",
              colWidths[2],
              "Subtype");
    wattroff(left_win, A_BOLD);
    whline(left_win, '-', leftTableWidth);

    // 计算显示范围
    INT32 maxDisplayRows = LINES - 4; // 保留顶部2行 + 底部2行

    // 绘制可见行
    for (size_t i = 0; i < (size_t) maxDisplayRows && (startRow + i) < visibleNodes.size(); ++i) {
        size_t actualIndex = startRow + i;
        TreeNode *node = visibleNodes[actualIndex];
        const DataModel &item = node->data;

        // 准备显示内容
        string displayName = item.getName();
        if (!node->children.empty()) {
            displayName += node->isExpanded ? " <" : " >";
        }

        // 处理超长文本
        const string clippedName = (displayName.length() > (size_t) colWidths[0])
            ? displayName.substr(0, (size_t) colWidths[0] - 3) + "..."
            : displayName;
        const string clippedType = (item.getType().length() > (size_t) colWidths[1])
            ? item.getType().substr(0, (size_t) colWidths[1] - 3) + ".."
            : item.getType();
        const string clippedSubtype = (item.getSubType().length() > (size_t) colWidths[2])
            ? item.getSubType().substr(0, (size_t) colWidths[2] - 3) + ".."
            : item.getSubType();

        // 高亮当前行
        if (i == (size_t) selectedRow) {
            wattron(left_win, A_REVERSE);
        }

        mvwprintw(left_win,
                  (int) i + 2,
                  0,
                  "%-*s | %-*s | %-*s",
                  colWidths[0],
                  clippedName.c_str(),
                  colWidths[1],
                  clippedType.c_str(),
                  colWidths[2],
                  clippedSubtype.c_str());

        if (i == (size_t) selectedRow) {
            wattroff(left_win, A_REVERSE);
        }
    }

    // 底部UI
    mvwhline(left_win, LINES - 2, 0, '-', leftTableWidth);
    wattron(left_win, A_DIM);
    mvwprintw(left_win,
              LINES - 1,
              2,
              "Items: %zu | Pos: %d-%d",
              visibleNodes.size(),
              scrollOffset,
              scrollOffset + (LINES - 5));
    wattroff(left_win, A_DIM);

    wrefresh(left_win);
}

void BiosCliView::drawPanel() {
    werase(right_win);

    // 边框
    rightPanelWidth = 35;

    // 标题
    wvline(right_win, '|', LINES - 1);

    wattron(right_win, A_BOLD);
    mvwprintw(right_win, 0, 2, "Volume Details");
    wattroff(right_win, A_BOLD);

    mvwhline(right_win, 1, 1, '-', rightPanelWidth);

    // 获取当前选中节点数据
    if (visibleNodes.empty()) {
        wrefresh(right_win);
        return;
    }
    size_t actualIndex = (size_t) (scrollOffset + selectedRow);
    TreeNode *node = visibleNodes[actualIndex];
    Volume *vol = node->data.getVolume();
    vol->setInfoStr();

    string info = vol->getInfoText();

    int y = 2;

    // 通用信息显示
    mvwprintw(right_win, y++, 2, "%-6s: 0x%08llX", "Offset", vol->getOffset());
    mvwprintw(right_win, y++, 2, "%-6s: 0x%08llX", "Length", vol->getSize());

    y++; // 信息字段前的空行

    // 信息字段的智能换行处理
    const int maxLineWidth = rightPanelWidth - 2; // 右侧边框占1列 + 缩进1列
    int currentLine = 0;
    size_t startPos = 0;
    size_t endPos = 0;

    // 分割字符串并添加缩进
    while (startPos < info.length() && y < LINES - 4) {
        // 查找换行符或最大宽度
        endPos = info.find('\n', startPos);
        if (endPos == string::npos)
            endPos = info.length();

        // 处理单行分段
        while (startPos < endPos) {
            size_t chunkEnd = startPos + (size_t) maxLineWidth;
            chunkEnd = std::min(chunkEnd, endPos);

            // 添加缩进（首行缩进2列，后续行缩进4列）
            string line = (currentLine == 0) ? " " : " ";
            line += info.substr(startPos, chunkEnd - startPos);

            mvwprintw(right_win, y++, 1, "%s", line.c_str());
            currentLine++;

            startPos = chunkEnd;
            if (y >= LINES - 4)
                break;
        }

        startPos = endPos + 1; // 跳过换行符
        currentLine = 0;       // 重置行计数器
    }

    wrefresh(right_win);
}

void BiosCliView::setTreeData() {
    left_win = newwin(LINES, leftTableWidth, 0, 0);
    right_win = newwin(LINES, rightPanelWidth, 0, leftTableWidth);

    int ch;
    do {
        rebuildVisibleList(); // 刷新可见列表
        drawTable((UINT64) scrollOffset);
        drawPanel();
        ch = getch();

        switch (ch) {
        case KEY_UP:
            if (selectedRow > 0) {
                --selectedRow;
            } else if (scrollOffset > 0) {
                --scrollOffset;
            }
            break;

        case KEY_DOWN:
            if (selectedRow < (LINES - 5) && (size_t) (scrollOffset + selectedRow + 1) < visibleNodes.size()) {
                ++selectedRow;
            } else {
                ++scrollOffset;
            }
            break;

        case '\n': // Enter键切换展开状态
        {
            const int actualIndex = scrollOffset + selectedRow;
            if (actualIndex >= 0 && (size_t) actualIndex < visibleNodes.size()) {
                TreeNode *node = visibleNodes[(size_t) actualIndex];
                if (!node->children.empty()) {
                    node->isExpanded = !node->isExpanded;
                    rebuildVisibleList();
                    // 保持滚动位置
                    selectedRow = std::min(selectedRow, (int) visibleNodes.size() - 1);
                }
            }
            break;
        }
        case 'Q':
        case 'q':
            ch = 'q';
            break;
        }

        // 边界检查
        scrollOffset = std::clamp(scrollOffset, 0, std::max(0, (int) visibleNodes.size() - (LINES - 4)));
        selectedRow = std::clamp(selectedRow, 0, std::min(LINES - 5, (int) visibleNodes.size() - 1));

    } while (ch != 'q');

    delwin(right_win);
    delwin(left_win);
}

void BiosCliView::ReorganizeVolume(Volume *volume) {
    if (Volume *newVolume = volume->Reorganize(); newVolume != nullptr) {
        safeDelete(volume);
        volume = newVolume;
    }
    for (auto childVolume : volume->ChildVolume) {
        ReorganizeVolume(childVolume);
    }
}

BiosCliData::~BiosCliData() {
    for (auto *volume : VolumeDataList) {
        safeDelete(volume);
    }
    delete OverviewVolume;
    if (!IFWI_exist) {
        BiosImage->ChildVolume.clear();
        safeDelete(BiosImage);
    }
    delete OverviewImageModel;
}
