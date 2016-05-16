#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX 255
/*
	Code for Person class
**/
// Declaration of class Person
struct Person_class {
	void* super;
	void (* toString) (void* this, char*, int);
};
// Object template
struct Person {
	struct Person_class* class;
	char* name;
};

// Declaration of instance methods
void Person_toString (void* thisv, char* buf, int size) {
	struct Person* this = thisv;
	snprintf(buf, size, "Name: %s", this->name);
}
// Static allocation and initializaiton of Person object
struct Person_class Person_class_obj =  {NULL,Person_toString};


// Constructor method
struct Person* new_Person(char* str) {
	struct Person* obj = malloc (sizeof (struct Person));
	obj->name = strdup(str);
	obj->class = &Person_class_obj;
	return obj;
}

/*
	Code for Student. Student is a subclass of Person
**/
// Student jump table
struct Student_class {
	struct Person_class* super;
	void (*toString) (void* this, char*, int);
};
// Student object template
struct Student {
	struct Student_class* class;
	char* name;
	int sid;
};

// Declaraiton of subclass instance methods
void Student_toString(void* thisv, char* buf, int size) {
	struct Student* this = thisv;
	char string[size];
	this->class->super->toString(this, string, size);
	snprintf(buf, size, "%s, SID: %i", string, this->sid);
}

struct Student_class Student_class_obj = {&Person_class_obj, Student_toString};

// Student constructor
struct Student* new_Student(char* name, int id) {
	struct Student* obj = malloc(sizeof(struct Student));
	obj->class = &Student_class_obj;
	obj->name = strdup(name);
	obj->sid = id;
	return obj;
}

// Static print method
void print(void* pv) {
	struct Person* p = pv;
	char buffer[MAX];
	p->class->toString(p, buffer, sizeof(buffer));
	printf("%s\n", buffer);

}
// Main
int main(int argc, char** argv) {
	struct Person* people[2] = {new_Person("Alex"), (struct Person*) new_Student("Alice", 300)};
	for (int i = 0; i < 2; i++)
		print(people[i]);
}