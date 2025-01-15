import os
python_c=[]
file_target=[]
def find_files_containing_string(directory, search):
    for root, dirs, files in os.walk(directory):
        for file in files:
            file_path = os.path.join(root, file)
            try:
                with open(file_path, 'r') as f:
                    for line in f:
                        if search in line:
                            if file_path.endswith((".cpp",".c",".cc")):
                                #print("hasstring:" + file_path)
                                file_target.append(file_path)
                                break;


            except UnicodeDecodeError:
                continue


def find_function_in_py(search1,file_target):
    count =0
    for file in file_target:
        with open(file,'r')as f:
            for line in f:
                if search1 in line :
                    print("hasstring:" + file)
                    count=count+1
                    break;
    return count

                
    

directory_to_search = '/home/dingqiuran/test/pytorch'
search1 = 'PyMethodDef'
search='Python.h'
find_files_containing_string(directory_to_search, search)
print(f"the file contain python.h :{len(file_target)}")
print("Among these files contain pymethoddef are as follow:")
print(f"the number is {find_function_in_py(search1,file_target)}")