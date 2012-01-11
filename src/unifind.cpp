#include <boost/pending/disjoint_sets.hpp>
#include<iostream>
#include <string>
#include <map>

template <typename DisjointSet>
struct test_disjoint_set {
  static void do_test()
  {
    // The following tests are pretty lame, just a basic sanity check.
    // Industrial strength tests still need to be written.

    std::size_t elts[] = { 0, 1, 2, 3 ,7, 8, 9, 10};
    const int N = sizeof(elts)/sizeof(std::size_t);

//    std::cout << "N:" << N << std::endl;
    DisjointSet ds(10);

    ds.make_set(elts[0]);
//    std::cout << ds.find_set(0) << std::endl;
    ds.make_set(elts[1]);
//    std::cout << ds.find_set(1) << std::endl;
    ds.make_set(elts[2]);
//    std::cout << ds.find_set(2) << std::endl;
    ds.make_set(elts[3]);
//    std::cout << ds.find_set(3) << std::endl;
    ds.make_set(elts[4]);
//    std::cout << ds.find_set(4) << std::endl;
    ds.make_set(elts[5]);
 //   std::cout << ds.find_set(5) << std::endl;
    ds.make_set(elts[6]);
//    std::cout << ds.find_set(6) << std::endl;
    ds.make_set(elts[7]);

    std::cout << ds.count_sets(elts, elts+N) << std::endl;

    std::cout << "find set: " << ds.find_set(0) << std::endl;
    if(ds.find_set(0) != ds.find_set(1))
	std::cout << "1" << std::endl;
    if(ds.find_set(0) != ds.find_set(2))
	std::cout << "2" << std::endl;
    if(ds.find_set(0) != ds.find_set(3))
	std::cout << "3" << std::endl;
    if(ds.find_set(1) != ds.find_set(2))
	std::cout << "4" << std::endl;
    if(ds.find_set(1) != ds.find_set(3))
	std::cout << "5" << std::endl;
    if(ds.find_set(2) != ds.find_set(3))
	std::cout << "6" << std::endl;


    ds.union_set(0, 1);
    ds.union_set(2, 3);
    ds.union_set(3, 7);
    ds.make_set(1);

    if(ds.find_set(0) != ds.find_set(3))
	std::cout << "7" << std::endl;
    int a = ds.find_set(0);
    if(a == ds.find_set(1))
	std::cout << "8" << std::endl;
    int b = ds.find_set(2);
    if(b == ds.find_set(3))
	std::cout << "10" << std::endl;
    if(ds.find_set(3) == ds.find_set(7))
	    std::cout << "get it!" << std::endl;
    ds.link(a, b);
    if(ds.find_set(a) == ds.find_set(b));
	std::cout << "13" << std::endl;
    std::cout << ds.count_sets(elts, elts + N) << std::endl;

    ds.normalize_sets(elts, elts + N);
    ds.compress_sets(elts, elts + N);
    std::cout <<  ds.count_sets(elts, elts + N) << std::endl;
  }
};

int
main()
{
	std::map<int, int> test;
	test.insert(std::pair<int,int>(1,2));
	test.insert(std::pair<int, int>(1,5));
	std::cout << test.find(1)->second << std::endl;
	++(test.find(1)->second);
	std::cout << test.find(1)->second << std::endl;
	std::cout << "end\n";

  using namespace boost;
  {
    typedef
      disjoint_sets_with_storage<identity_property_map, identity_property_map,
      find_with_path_halving> ds_type;
    test_disjoint_set<ds_type>::do_test();
  }/*
  {
    typedef
      disjoint_sets_with_storage<identity_property_map, identity_property_map,
      find_with_full_path_compression> ds_type;
    test_disjoint_set<ds_type>::do_test();
  }*/
  return 1;
}

