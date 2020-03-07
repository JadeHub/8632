/*

void hash_test()
{
	typedef struct Bar
	{
		kname_t name;
		hash_tbl_item_t hash_item;
	}Bar_t;

	hash_tbl_t* ht = hash_tbl_create(256);

	Bar_t b1;
	kname_set("B 1", &b1.name);
	hash_tbl_add(ht, 1, &b1.hash_item);

	Bar_t b2;
	kname_set("B 511", &b2.name);
	hash_tbl_add(ht, 511, &b2.hash_item);

	Bar_t b3;
	kname_set("B 2048", &b3.name);
	hash_tbl_add(ht, 2048, &b3.hash_item);

	hash_tbl_item_t* i = hash_tbl_find(ht, 511);

	if (i)
	{

		Bar_t* test = container_of(i, Bar_t, hash_item);
		printf("found %s\n", test->name.str);
	}

	Bar_t* t2 = hash_tbl_lookup(ht, 2048, Bar_t, hash_item);
	if (t2)
		printf("Found %s\n", t2->name.str);
}

void list_test()
{
	typedef struct Foo
	{
		int bar;
		list_head_t list;
	}Foo_t;

	//Declare a list

	list_head_t my_list;
	INIT_LIST_HEAD(&my_list);

	printf("Empty = %d\n", list_empty(&my_list));

	Foo_t f1;
	f1.bar = 1;
	list_add(&f1.list, &my_list);


	printf("Empty = %d\n", list_empty(&my_list));

	Foo_t f2;
	f2.bar = 2;
	list_add(&f2.list, &my_list);

	Foo_t f3;
	f3.bar = 3;
	list_add(&f3.list, &my_list);

	list_head_t* item;
	list_for_each_rev(item, &my_list)
	{
		Foo_t* foo = list_entry(item, Foo_t, list);
	//	printf("Foo %d\n", foo->bar);
		if(foo->bar == 3)
		{
			list_delete(&foo->list);
			break;
		}
	}

	Foo_t* f;
	list_for_each_entry(f, &my_list, list)
	{
		printf("Foo %d\n", f->bar);
	}
}


*/