## 1. 插入排序

核心思想：从数组的第二数开始，从后向前遍历前面的子数组，数组位置后移，插入到合适的位置中

- 需要保存数组后面一位数，然后对整个数组进行向后移的操作

```c
/* 
 * 插入排序，从小到大
 * 从第二个数开始，选出一个key，插入到介于他大小的位置上去，
 * （他每次排的子数组都是有序的）
 */
void insertSort(int arr[], int n)
{
    for(int i = 1; i < n; i++)
    {
        int key = arr[i];
        int j = i-1;
        while(j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j+1] = key;
    }
}
```

## 2. 选择排序

核心思想：从第一个索引到 n-1个索引，选出后面子数组最小的值与索引处进行交换

```c
/* 
 * 选择排序，从小到大
 * 每次以第一个数为最小值，遍历后序子串，找比他更小的索引
 * 每次找到索引后，与第一个数进行交换
 */
void SelectSort(int arr[], int n)
{
    for(int i = 0; i < n-1; i++)
    {
        int min_index = i;
        for(int j = i+1; j < n; j++)
        {
            if(arr[j] < arr[min_index])
                min_index = j;
        }
        if(min_index != i)
        {
            int temp = arr[i];
            arr[i] = arr[min_index];
            arr[min_index] = temp;
        }
        
    }
}
```

## 3. 快速排序

核心思想：每一次排序都以某个数为基准，比他小的在左边，比他大的在右边；每一次排序划分两个区间，直到不能再分为止

```c
void QuickSort(int arr[], int low, int high)
{
    if(low < high)
    {
        int CompareValue = arr[high];    // 每次排序就以这个为基准，进行分区间
        int i = low - 1;
        for(int j = low; j <= high - 1; j++)    // 遍历 [low, high-1] 的区间，比 CompareKey 小的要放到前面 i 位置上
        {
            if(arr[j] < CompareValue)
            {
                i++;
                int temp = arr[j];
                arr[j] = arr[i];
                arr[i] = temp;
            }
        }
        arr[high] = arr[i + 1];
        arr[i+1] = CompareValue;

        int partitionIndex = i+1;
        QuickSort(arr, low, partitionIndex - 1);
        QuickSort(arr, partitionIndex+1, high);
    }
}
```

## 4. 冒泡排序

- 前一个比后一个大就两两交换，把最大的那个数冒泡到最后面，每一轮冒泡一个

```c
void PaoSort(int arr[], int n)
{
    for(int i = 0; i < n-1; i++)			// 冒泡的轮数
    {
        for(int j = 0; j + 1 < n - i; j++)	// 进行冒泡的区间范围
        {
            if(arr[j] > arr[j+1])		// 前一个比后一个大，就进行两两交换，开始冒泡
            {
                int temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}
```
