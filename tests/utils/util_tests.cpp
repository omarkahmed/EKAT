#include <catch2/catch.hpp>

#include "ekat/util/ekat_string_utils.hpp"
#include "ekat/ekat_pack.hpp"
#include "ekat/kokkos/ekat_kokkos_meta.hpp"
#include "ekat/ekat_type_traits.hpp"

#include "ekat_test_config.h"

namespace {

TEST_CASE("precision", "util") {
  CHECK_FALSE(ekat::is_single_precision<double>::value);
  CHECK(ekat::is_single_precision<float>::value);
}

TEST_CASE("type_traits", "") {
  using namespace ekat;
  REQUIRE(std::is_same<ekat::ValueType<double**&>::type,double>::value);
  REQUIRE(std::is_same<ekat::ValueType<double*[3]>::type,double>::value);
  REQUIRE(std::is_same<ekat::ValueType<double[2][3]>::type,double>::value);

  // Check meta-util to get rank and dynamic rank of a raw MD array
  REQUIRE(ekat::GetRanks<double[2][3]>::rank==2);
  REQUIRE(ekat::GetRanks<double[2][3]>::rank_dynamic==0);
  REQUIRE(ekat::GetRanks<double*[2][3]>::rank==3);
  REQUIRE(ekat::GetRanks<double*[2][3]>::rank_dynamic==1);
  REQUIRE(ekat::GetRanks<double**[2][3]>::rank==4);
  REQUIRE(ekat::GetRanks<double**[2][3]>::rank_dynamic==2);
}

// This is just a compilation test.
TEST_CASE("Unmanaged", "ekat::ko") {
  using ekat::util::Unmanaged;

  {
    typedef Kokkos::View<double*> V;
    V v("v", 10);
    typedef Unmanaged<V> VUm;
    VUm v_um(v);
    static_assert( ! V::traits::memory_traits::Unmanaged, "Um");
    static_assert(VUm::traits::memory_traits::Unmanaged, "Um");
  }

  {
    typedef Kokkos::View<ekat::pack::Pack<double, EKAT_TEST_PACK_SIZE>***,
                         Kokkos::LayoutLeft,
                         Kokkos::HostSpace,
                         Kokkos::MemoryTraits<Kokkos::RandomAccess> >
      V;
    V v("v", 2, 3, 4);
    typedef Unmanaged<V> VUm;
    static_assert(VUm::traits::memory_traits::RandomAccess, "Um");
    static_assert(VUm::traits::memory_traits::Unmanaged, "Um");
    VUm v_um(v);
    typedef Unmanaged<VUm> VUmUm;
    static_assert(VUmUm::traits::memory_traits::RandomAccess, "Um");
    static_assert(VUmUm::traits::memory_traits::Unmanaged, "Um");
    static_assert( ! VUmUm::traits::memory_traits::Atomic, "Um");
    static_assert( ! VUmUm::traits::memory_traits::Aligned, "Um");
    static_assert( ! VUmUm::traits::memory_traits::Restrict, "Um");
    VUmUm v_umum(v);
  }

  {
    typedef Kokkos::View<ekat::pack::Pack<int, EKAT_TEST_PACK_SIZE>[10],
                         Kokkos::HostSpace,
                         Kokkos::MemoryTraits<Kokkos::Atomic | Kokkos::Aligned | Kokkos::Restrict> >
      V;
    static_assert( ! V::traits::memory_traits::Unmanaged, "Um");
    V v("v");
    typedef Unmanaged<V>::const_type CVUm;
    static_assert(CVUm::traits::memory_traits::Atomic, "Um");
    static_assert(CVUm::traits::memory_traits::Aligned, "Um");
    static_assert(CVUm::traits::memory_traits::Restrict, "Um");
    static_assert(CVUm::traits::memory_traits::Unmanaged, "Um");

    using Kokkos::Impl::ViewMapping;
    static_assert(ViewMapping<CVUm::traits, V::traits, void>::is_assignable,
                  "CVUm <- V");
    static_assert( ! ViewMapping<V::traits, CVUm::traits, void>::is_assignable,
                  "V </- CVUm");
    static_assert(ViewMapping<CVUm::traits, Unmanaged<V>::traits, void>::is_assignable,
                  "CVUm <- VUm");
    static_assert( ! ViewMapping<Unmanaged<V>::traits, CVUm::traits, void>::is_assignable,
                  "VUm </- CVUm");
    CVUm cv_um(v);
  }
}

TEST_CASE("string","string") {
  using namespace ekat;

  util::CaseInsensitiveString cis1 = "field_1";
  util::CaseInsensitiveString cis2 = "fIeLd_1";
  util::CaseInsensitiveString cis3 = "field_2";
  util::CaseInsensitiveString cis4 = "feld_1";

  REQUIRE (cis1==cis2);
  REQUIRE (cis1!=cis3);
  REQUIRE (cis4<=cis1);
  REQUIRE (cis4<cis1);

  std::string my_str  = "item 1  ; item2;  item3 ";
  std::string my_list = "item1;item2;item3";

  util::strip(my_str,' ');
  REQUIRE(my_str==my_list);

  auto items = util::split(my_list,';');
  REQUIRE(items.size()==3);
  REQUIRE(items[0]=="item1");
  REQUIRE(items[1]=="item2");
  REQUIRE(items[2]=="item3");

  // Jaro and Jaro-Winkler similarity tests

  // Benchmark list (including expected similarity values) from Winkler paper
  //  https://www.census.gov/srd/papers/pdf/rrs2006-02.pdf
  // Note: Winkler clamps all values below 0.7 to 0. I don't like that,
  //       so I had to remove some entries.

  //                          LHS         RHS       JARO   JARO-WINKLER
  using entry_type = std::tuple<std::string,std::string,double, double>;

  std::vector<entry_type> benchmark =
    {
      entry_type{ "shackleford", "shackelford", 0.970, 0.982 },
      entry_type{ "dunningham" , "cunnigham"  , 0.896, 0.896 },
      entry_type{ "nichleson"  , "nichulson"  , 0.926, 0.956 },
      entry_type{ "jones"      , "johnson"    , 0.790, 0.832 },
      entry_type{ "massey"     , "massie"     , 0.889, 0.933 },
      entry_type{ "abroms"     , "abrams"     , 0.889, 0.922 },
      entry_type{ "jeraldine"  , "geraldine"  , 0.926, 0.926 },
      entry_type{ "marhta"     , "martha"     , 0.944, 0.961 },
      entry_type{ "michelle"   , "michael"    , 0.869, 0.921 },
      entry_type{ "julies"     , "julius"     , 0.889, 0.933 },
      entry_type{ "tanya"      , "tonya"      , 0.867, 0.880 },
      entry_type{ "dwayne"     , "duane"      , 0.822, 0.840 },
      entry_type{ "sean"       , "susan"      , 0.783, 0.805 },
      entry_type{ "jon"        , "john"       , 0.917, 0.933 },
    };

  const double tol = 0.005;
  for (const auto& entry : benchmark) {
    const auto& s1 = std::get<0>(entry);
    const auto& s2 = std::get<1>(entry);
    double sj  = util::jaro_similarity(s1,s2);
    double sjw = util::jaro_winkler_similarity(s1,s2);

    const double sj_ex = std::get<2>(entry);
    const double sjw_ex = std::get<3>(entry);

    REQUIRE (std::fabs(sj-sj_ex)<tol);
    REQUIRE (std::fabs(sjw-sjw_ex)<tol);
  }

}

} // empty namespace
