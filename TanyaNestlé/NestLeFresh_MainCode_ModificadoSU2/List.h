

#pragma once
#ifndef LIST
#define LIST

#include "Arduino.h"

/*
In this modified version:

The class is now declared as a template class FIFO_List<T>, where T represents the data type of the elements in the list.
The Node struct is also templated to store elements of type T.
The push, pop, and other methods are updated to work with data of type T.
You can use this templated FIFO_List class to create FIFO lists of any data type, including structs,
by specifying the desired type when you create an instance of the class. For example, you can create a FIFO_List<MyStruct> to work with elements of type MyStruct.
*/

template <typename T>
class FIFO_List
{
public:
    FIFO_List();  // Constructor
    ~FIFO_List(); // Destructor

    void push(const T &data); // Add an element to the end of the list
    T pop();                  // Remove and return the element from the front of the list
    bool isEmpty() const;     // Check if the list is empty

private:
    struct Node
    {
        T data;
        Node *next;
    };

    Node *front;
    Node *rear;
};

template <typename T>
FIFO_List<T>::FIFO_List() : front(nullptr), rear(nullptr) {}

template <typename T>
FIFO_List<T>::~FIFO_List()
{
    while (!isEmpty())
    {
        pop();
    }
}

template <typename T>
void FIFO_List<T>::push(const T &data)
{
    Node *newNode = new Node;
    newNode->data = data;
    newNode->next = nullptr;

    if (isEmpty())
    {
        front = rear = newNode;
    }
    else
    {
        rear->next = newNode;
        rear = newNode;
    }
}

template <typename T>
T FIFO_List<T>::pop()
{
    if (isEmpty())
    {
        // Handle empty list (e.g., return a default-constructed value or throw an exception)
        return T(); // Default-constructed value for type T
    }

    Node *temp = front;
    T data = temp->data;
    front = front->next;
    delete temp;

    if (front == nullptr)
    {
        rear = nullptr;
    }

    return data;
}

template <typename T>
bool FIFO_List<T>::isEmpty() const
{
    return front == nullptr;
}

/*
In this Ordered_List class:
The elements are added to the beginning of the list using the push method, which makes the newest element the first in the list.
The remove method searches for and removes the first occurrence of the specified element in the list.
The search method checks whether a specific element is present in the list.
The class maintains a singly linked list (Node) where each node contains an element of type T.
You can use this Ordered_List class to store and manage elements in the order they are received, search for specific elements,
and remove elements as needed.
*/

template <typename T>
class Ordered_List
{
public:
    Ordered_List();  // Constructor
    ~Ordered_List(); // Destructor

    void push(const T &data);   // Add an element to the end of the list
    bool remove(const T &data); // Remove the first occurrence of the element
    bool search(const T &data); // Search for the element

    bool isEmpty() const; // Check if the list is empty

private:
    struct Node
    {
        T data;
        Node *next;
    };

    Node *head;
};

template <typename T>
Ordered_List<T>::Ordered_List() : head(nullptr) {}

template <typename T>
Ordered_List<T>::~Ordered_List()
{
    while (!isEmpty())
    {
        Node *temp = head;
        head = head->next;
        delete temp;
    }
}

template <typename T>
void Ordered_List<T>::push(const T &data)
{
    Node *newNode = new Node;
    newNode->data = data;
    newNode->next = head;
    head = newNode;
}

template <typename T>
bool Ordered_List<T>::remove(const T &data)
{
    Node *current = head;
    Node *previous = nullptr;

    while (current != nullptr)
    {
        if (current->data == data)
        {
            if (previous != nullptr)
            {
                previous->next = current->next;
            }
            else
            {
                head = current->next;
            }
            delete current;
            return true;
        }
        previous = current;
        current = current->next;
    }

    return false; // Element not found
}

template <typename T>
bool Ordered_List<T>::search(const T &data)
{
    Node *current = head;
    while (current != nullptr)
    {
        if (current->data == data)
        {
            return true; // Element found
        }
        current = current->next;
    }
    return false; // Element not found
}

template <typename T>
bool Ordered_List<T>::isEmpty() const
{
    return head == nullptr;
}

/*
In this Linked_List class:

Each element is represented by a Node struct, which contains an index and data of type T.
The insert method inserts elements in the list in ascending order based on the index.
The get method retrieves data by searching for the specified index.
The remove method removes elements by index.
The search method checks if a specific index is present in the list.
This implementation ensures that elements are ordered by index and handles memory management by deallocating nodes in the destructor.
You can use this Linked_List class to store and manage elements with byte indices and data of an unspecified data type.
*/

template <typename T>
class Linked_List
{
public:
    Linked_List();  // Constructor
    ~Linked_List(); // Destructor

    void insert(int index, const T &data); // Insert an element at a specific index
    bool get(int index, T &data);          // Retrieve data by index
    bool remove(int index);                // Remove an element by index
    bool search(int index);                // Search for an index in the list
    int maxIndex() const;
    int countElements() const;

private:
    struct Node
    {
        int index;
        T data;
        Node *next;
    };

    Node *head;
};

template <typename T>
Linked_List<T>::Linked_List() : head(nullptr) {}

template <typename T>
Linked_List<T>::~Linked_List()
{
    while (head != nullptr)
    {
        Node *temp = head;
        head = head->next;
        delete temp;
    }
}

template <typename T>
void Linked_List<T>::insert(int index, const T &data)
{
    Node *newNode = new Node;
    newNode->index = index;
    newNode->data = data;

    if (head == nullptr || index < head->index)
    {
        // Insert at the beginning
        newNode->next = head;
        head = newNode;
    }
    else
    {
        // Insert at the appropriate position
        Node *current = head;
        while (current->next != nullptr && current->next->index < index)
        {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}

template <typename T>
bool Linked_List<T>::get(int index, T &data)
{
    Node *current = head;
    while (current != nullptr && current <= index)
    {
        if (current->index == index)
        {
            data = current->data;
            return true; // Index found
        }
        current = current->next;
    }
    return false; // Index not found
}

template <typename T>
bool Linked_List<T>::remove(int index)
{
    if (head == nullptr)
    {
        return false; // List is empty
    }

    if (head->index == index)
    {
        // Remove the first element
        Node *temp = head;
        head = head->next;
        delete temp;
        return true; // Index found and removed
    }

    Node *current = head;
    while (current->next != nullptr && current->next->index != index)
    {
        current = current->next;
    }

    if (current->next != nullptr)
    {
        // Remove an element from the middle
        Node *temp = current->next;
        current->next = temp->next;
        delete temp;
        return true; // Index found and removed
    }

    return false; // Index not found
}

template <typename T>
bool Linked_List<T>::search(int index)
{
    Node *current = head;
    while (current != nullptr)
    {
        if (current->index == index)
        {
            return true; // Index found
        }
        current = current->next;
    }
    return false; // Index not found
}

template <typename T>
int Linked_List<T>::maxIndex() const
{
    int max = -1; // Initialize with a value lower than the lowest possible index
    Node *current = head;
    while (current != nullptr)
    {
        if (current->index > max)
        {
            max = current->index;
        }
        current = current->next;
    }
    return max;
}

template <typename T>
int Linked_List<T>::countElements() const
{
    int count = 0;
    Node *current = head;
    while (current != nullptr)
    {
        if (current->index != 0)
        {
            count++;
        }
        current = current->next;
    }
    return count;
}

/*
This Simple_Linked_List class behaves similarly to the Linked_List class but without the need for indices.
It appends new elements to the end of the list, and you can count the elements using countElements().
*/

template <typename T>
class Simple_Linked_List
{
public:
    Simple_Linked_List(int _maxIndex); // Constructor
    Simple_Linked_List(); // Constructor
    ~Simple_Linked_List();             // Destructor

    bool insert(const T &data);   // Add an element to the end of the list
    bool get(int index, T &data); // Retrieve data by index
    bool get(T &data);            // Retrieve data by index
    bool remove(int index);       // Remove an element by index
    bool remove();                // Remove an element by index
    int countElements() const;    // Count the number of elements in the list
    void clearList();             // Deletes All the list
    void SetMaxIndex(int _maxIndex);

private:
    struct Node
    {
        T data;
        Node *next;
    };

    Node *head;
    byte maxIndex;
};

template <typename T>
Simple_Linked_List<T>::Simple_Linked_List(int _maxIndex) : head(nullptr)
{
    if (_maxIndex > 0)
        SetMaxIndex(_maxIndex);
    else
        SetMaxIndex(32);
}

template <typename T>
Simple_Linked_List<T>::Simple_Linked_List() : head(nullptr)
{
        SetMaxIndex(32);
}

template <typename T>
Simple_Linked_List<T>::~Simple_Linked_List()
{
    clearList();
}

template <typename T>
void Simple_Linked_List<T>::clearList()
{
    while (head != nullptr)
    {
        Node *temp = head;
        head = head->next;
        delete temp;
    }
}

template <typename T>
bool Simple_Linked_List<T>::insert(const T &data)
{
    if (countElements() <= this->maxIndex)
    {
        Node *newNode = new Node;
        newNode->data = data;
        newNode->next = nullptr;

        if (head == nullptr)
        {
            head = newNode;
        }
        else
        {
            Node *current = head;
            while (current->next != nullptr)
            {
                current = current->next;
            }
            current->next = newNode;
        }
        return true;
    }
    else
    {
        return false;
    }
}

template <typename T>
bool Simple_Linked_List<T>::get(int index, T &data)
{
    if (index < 0 || index > countElements())
    {
        return false; // Invalid index
    }

    Node *current = head;
    int currentIndex = 0;

    while (current != nullptr)
    {
        if (currentIndex == index)
        {
            data = current->data;
            return true; // Index found
        }
        current = current->next;
        currentIndex++;
    }

    return false; // Index not found
}

template <typename T>
bool Simple_Linked_List<T>::get(T &data)
{
    return get(0, data);
}

template <typename T>
bool Simple_Linked_List<T>::remove(int index)
{
    if (index < 0 || index > countElements())
    {
        return false; // Invalid index
    }

    if (index == 0)
    {
        // Remove the first element
        if (head != nullptr)
        {
            Node *temp = head;
            head = head->next;
            delete temp;
            return true;
        }
        return false; // List is empty
    }

    Node *current = head;
    int currentIndex = 0;

    while (current != nullptr && currentIndex < index - 1)
    {
        current = current->next;
        currentIndex++;
    }

    if (current != nullptr && current->next != nullptr)
    {
        Node *temp = current->next;
        current->next = current->next->next;
        delete temp;
        return true;
    }

    return false; // Index not found
}

template <typename T>
bool Simple_Linked_List<T>::remove()
{
    return remove(0);
}

template <typename T>
int Simple_Linked_List<T>::countElements() const
{
    int count = 0;
    Node *current = head;
    while (current != nullptr)
    {
        count++;
        current = current->next;
    }
    return count;
}

template <typename T>
void Simple_Linked_List<T>::SetMaxIndex(int _maxIndex)
{
    if (_maxIndex > 0)
        this->maxIndex = _maxIndex;
}

#endif // !LIST