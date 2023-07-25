import os

def rename_files():
    # 获取当前目录
    current_dir = os.getcwd()
    
    # 遍历当前目录
    for file_name in os.listdir(current_dir):
        if file_name.endswith(".c"):
            # 生成新文件名
            new_file_name = file_name.replace(".c", ".cpp")
            
            # 重命名文件
            os.rename(file_name, new_file_name)
            
            print(f"文件 {file_name} 重命名为 {new_file_name}")

rename_files()
