#include <catch2/catch.hpp>

#include "ekat/util/ekat_meta_utils.hpp"

// We can't define template alias inside the TEST_CASE block,
// since, upon macro expansion, those blocks are at function scope.

template<typename T>
using TestVec = std::vector<T>;

TEST_CASE ("meta_utils") {
  using namespace ekat;

  using L1 = TypeList<int,float,double>;
  using L2 = TypeList<char,void>;
  using L3 = TypeList<int,char>;
  using L4 = TypeList<int,float,double>;
  using L5 = TypeList<std::string,std::string,std::string>;

  SECTION ("type_list") {
    // Check size is correct
    REQUIRE (std::is_same<L1::head,int>::value);
    constexpr int L1size = L1::size;
    REQUIRE (L1size==3);

    // Check concat of two lists equals list of union of their types
    using L1L2 = typename CatLists<L1,L2>::type;
    REQUIRE (std::is_same<L1L2,TypeList<int,float,double,char,void>>::value);
    constexpr int L1L2size = L1L2::size;
    REQUIRE (L1L2size==5);

    // Check unique returns the expected value
    using L1L2 = typename CatLists<L1,L2>::type;
    using L1L3 = typename CatLists<L1,L3>::type;

    REQUIRE (UniqueTypeList<L1L2>::value);
    REQUIRE (not UniqueTypeList<L1L3>::value);

    // Check we can find the first entry of a type in a list (if found)
    constexpr auto f_pos = FirstOf<float,L1>::pos;
    constexpr auto v_pos = FirstOf<void,L1>::pos;

    REQUIRE (f_pos==1);
    REQUIRE (v_pos==-1);

    // Check access of type list
    REQUIRE (std::is_same<typename AccessList<L1,0>::type,int>::value);
    REQUIRE (std::is_same<typename AccessList<L1,1>::type,float>::value);
    REQUIRE (std::is_same<typename AccessList<L1,2>::type,double>::value);
  }

  SECTION ("type_map") {

    // Maps int/float/double to a string
    using map_t = TypeMap<L4,L5>;

    // Check map size
    constexpr auto map_size = map_t::size;
    REQUIRE (map_size==3);

    // Check all types in L4 (and only them) are present
    REQUIRE (map_t::has_t<int>());
    REQUIRE (map_t::has_t<float>());
    REQUIRE (map_t::has_t<double>());
    REQUIRE (not map_t::has_t<char>());

    // Check all types in the map are mapped to a string.
    // If type not in the string, return NotFound.
    using NotFound = typename map_t::NotFound;
    REQUIRE (std::is_same<map_t::at_t<int>,std::string>::value);
    REQUIRE (std::is_same<map_t::at_t<float>,std::string>::value);
    REQUIRE (std::is_same<map_t::at_t<double>,std::string>::value);
    REQUIRE (std::is_same<map_t::at_t<char>,NotFound>::value);

    // Create a map, and fill the mapped values
    map_t map;
    map.at<int>() = "int";
    map.at<float>() = "float";
    map.at<double>() = "double";

    // Check the map was filled correctly
    REQUIRE (map.at<int>() == "int");
    REQUIRE (map.at<float>() == "float");
    REQUIRE (map.at<double>() == "double");

    // Check we can find the inserted values, and not others
    // Note: we need do specify <std::string>, otherwise the template
    //       resolution would use char[], which is not in the mapped
    //       values typelist, so it would return false.
    REQUIRE (map.has_v<std::string>("int"));
    REQUIRE (map.has_v<std::string>("float"));
    REQUIRE (map.has_v<std::string>("double"));
    REQUIRE (not map.has_v<std::string>("char"));
    REQUIRE (not map.has_v(2));
  }

  SECTION ("apply_template") {
    // Given a template type, instantiate it over all types in a type list
    using Templates = typename ApplyTemplate<TestVec,L1>::type;

    REQUIRE (std::is_same<typename AccessList<Templates,0>::type,TestVec<int>>::value);
    REQUIRE (std::is_same<typename AccessList<Templates,1>::type,TestVec<float>>::value);
    REQUIRE (std::is_same<typename AccessList<Templates,2>::type,TestVec<double>>::value);
  }

  SECTION ("list_for") {
    // Test for loop over a list of types
    using K = L1;
    using V = typename ApplyTemplate<TestVec,K>::type;
    using map_t = TypeMap<K,V>;

    // Create a type map from int/float/double to std::vector, and fill it.
    map_t map;
    map.at<int>().resize(2);
    map.at<float>().resize(3);
    map.at<double>().resize(1);

    // Check the sum of sizes of vectors is 6, by using a for loop over type list
    int size = 0;
    TypeListFor<K>([&](auto t) {
      const auto& m = map.at<decltype(t)>();
      size += m.size();
    });

    REQUIRE (size==6);
  }
}
