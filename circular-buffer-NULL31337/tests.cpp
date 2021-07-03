#include "circular_buffer.h"
#include <utility>
#include <gtest/gtest.h>
#include "test-helpers/element.h"
#include "test-helpers/fault_injection.h"

using container = circular_buffer<element>;

template <typename C, typename T>
void mass_push_back(C& c, std::initializer_list<T> elems)
{
    for (T const& e : elems)
        c.push_back(e);
}

template <typename It, typename T>
void expect_eq(It i1, It e1, std::initializer_list<T> elems)
{
    auto i2 = elems.begin(), e2 = elems.end();

    for (;;)
    {
        if (i1 == e1 || i2 == e2)
        {
            EXPECT_TRUE(i1 == e1 && i2 == e2);
            break;
        }

        EXPECT_EQ(*i2, *i1);
        ++i1;
        ++i2;
    }
}

template <typename C, typename T>
void expect_eq(C const& c, std::initializer_list<T> elems)
{
    expect_eq(c.begin(), c.end(), elems);
}

template <typename C, typename T>
void expect_reverse_eq(C const& c, std::initializer_list<T> elems)
{
    expect_eq(c.rbegin(), c.rend(), elems);
}

TEST(correctness, default_ctor)
{
    element::no_new_instances_guard g;
    container c;
    EXPECT_EQ(0u, c.size());
    EXPECT_EQ(true, c.empty());
}

TEST(correctness, copy_ctor)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});

    container c2 = c;
    expect_eq(c2, {1, 2, 3, 4});
}

TEST(correctness, copy_ctor_empty)
{
    element::no_new_instances_guard g;
    container c;

    container c2 = c;
    EXPECT_EQ(0u, c.size());
    EXPECT_EQ(true, c.empty());
}

TEST(correctness, assignment_operator_1)
{
    element::no_new_instances_guard g;
    container c1, c2;
    mass_push_back(c1, {1, 2, 3, 4});
    mass_push_back(c2, {5, 6, 7, 8});

    c1 = c2;
    expect_eq(c1, {5, 6, 7, 8});
}

TEST(correctness, assignment_operator_2)
{
    element::no_new_instances_guard g;
    container c1, c2;
    mass_push_back(c1, {1, 2, 3, 4});

    c1 = c2;
    EXPECT_EQ(0u, c1.size());
    EXPECT_EQ(true, c1.empty());
}

TEST(correctness, assignment_operator_3)
{
    element::no_new_instances_guard g;
    container c1, c2;
    mass_push_back(c2, {5, 6, 7, 8});

    c1 = c2;
    expect_eq(c1, {5, 6, 7, 8});
}

TEST(correctness, assignment_operator_4)
{
    element::no_new_instances_guard g;
    container c1, c2;

    c1 = c2;
    EXPECT_EQ(0u, c1.size());
    EXPECT_EQ(true, c1.empty());
}

TEST(correctness, size)
{
    element::no_new_instances_guard g;
    container c;
    EXPECT_EQ(0u, c.size());

    c.push_back(42);
    EXPECT_EQ(1u, c.size());

    c.push_back(42);
    EXPECT_EQ(2u, c.size());
}

TEST(correctness, subscription_1)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    EXPECT_EQ(1, c[0]);
    EXPECT_EQ(2, c[1]);
    EXPECT_EQ(3, c[2]);
    EXPECT_EQ(4, c[3]);
    c[2] = 5;
    EXPECT_EQ(5, c[2]);
}

TEST(correctness, subscription_2)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    EXPECT_EQ(3, std::as_const(c)[2]);
}

TEST(correctness, empty)
{
    element::no_new_instances_guard g;
    container c;
    EXPECT_TRUE(c.empty());
    c.push_back(1);
    EXPECT_FALSE(c.empty());
}

TEST(correctness, clear_1)
{
    element::no_new_instances_guard g;
    container c;
    c.push_back(1);
    c.clear();
    EXPECT_TRUE(c.empty());
}

TEST(correctness, clear_2)
{
    element::no_new_instances_guard g;
    container c;
    c.clear();
    EXPECT_TRUE(c.empty());
}

TEST(correctness, push_back)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    expect_eq(c, {1, 2, 3, 4});
    c.push_back(5);
    expect_eq(c, {1, 2, 3, 4, 5});
}

TEST(correctness, pop_back)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    expect_eq(c, {1, 2, 3, 4});
    c.pop_back();
    expect_eq(c, {1, 2, 3});
}

TEST(correctness, back)
{
    element::no_new_instances_guard g;
    container c;
    c.push_back(1);
    EXPECT_EQ(1, c.back());
    EXPECT_EQ(1, std::as_const(c).back());

    c.push_back(2);
    EXPECT_EQ(2, c.back());
    EXPECT_EQ(2, std::as_const(c).back());

    c.back() = 3;
    expect_eq(c, {1, 3});
}

TEST(correctness, push_front)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    expect_eq(c, {1, 2, 3, 4});
    c.push_front(5);
    expect_eq(c, {5, 1, 2, 3, 4});
}

TEST(correctness, pop_front)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    expect_eq(c, {1, 2, 3, 4});
    c.pop_front();
    expect_eq(c, {2, 3, 4});
}

TEST(correctness, front)
{
    element::no_new_instances_guard g;
    container c;
    c.push_back(1);
    EXPECT_EQ(1, c.front());
    EXPECT_EQ(1, std::as_const(c).front());

    c.push_front(2);
    EXPECT_EQ(2, c.front());
    EXPECT_EQ(2, std::as_const(c).front());

    c.front() = 3;
    expect_eq(c, {3, 1});
}

TEST(correctness, reserve)
{
    element::no_new_instances_guard g;
    container c;
    c.reserve(10);
    EXPECT_GE(c.capacity(), 10u);
}

TEST(correctness, iterators_default_ctor)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});

    container::const_iterator it1;
    container::iterator it2;

    it1 = c.begin() + 2;
    it2 = c.end() - 2;
    EXPECT_TRUE(it1 == it2);
}

TEST(correctness, iterators_comparison)
{
    element::no_new_instances_guard g;
    container c;
    
    EXPECT_TRUE(c.begin() == c.end());
    EXPECT_FALSE(c.begin() != c.end());

    EXPECT_FALSE(c.begin() < c.end());
    EXPECT_FALSE(c.begin() > c.end());
    EXPECT_TRUE(c.begin() <= c.end());
    EXPECT_TRUE(c.begin() >= c.end());
}

TEST(correctness, iterators_conversion)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    
    container::iterator it1 = c.begin();
    container::const_iterator it2 = it1;
    EXPECT_EQ(1, *it2);
    ++it2;
    EXPECT_EQ(2, *it2);
}

TEST(correctness, iterators_operators)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});

    container::iterator it = c.begin();
    EXPECT_EQ(1, *it);
    EXPECT_EQ(2, *++it);
    EXPECT_EQ(2, *it++);
    EXPECT_EQ(3, *it);
    EXPECT_EQ(3, *it--);
    EXPECT_EQ(1, *--it);
    
    it += 3;
    EXPECT_EQ(4, *it);
    it -= 2;
    EXPECT_EQ(2, *it);
    EXPECT_EQ(3, *(it + 1));
    EXPECT_EQ(3, *(1 + it));
    EXPECT_EQ(1, *(it - 1));
    
    EXPECT_EQ(3, c.end() - it);

    EXPECT_EQ(3, it[1]);
    EXPECT_EQ(1, it[-1]);
}

TEST(correctness, insert_1)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    container::iterator it = c.insert(c.begin(), 5);

    expect_eq(c, {5, 1, 2, 3, 4});
    EXPECT_TRUE(it == c.begin());
}

TEST(correctness, insert_2)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    container::iterator it = c.insert(c.begin() + 1, 5);

    expect_eq(c, {1, 5, 2, 3, 4});
    EXPECT_TRUE(it == c.begin() + 1);
}

TEST(correctness, insert_3)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    container::iterator it = c.insert(c.begin() + 2, 5);

    expect_eq(c, {1, 2, 5, 3, 4});
    EXPECT_TRUE(it == c.begin() + 2);
}

TEST(correctness, insert_4)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    container::iterator it = c.insert(c.begin() + 3, 5);

    expect_eq(c, {1, 2, 3, 5, 4});
    EXPECT_TRUE(it == c.begin() + 3);
}

TEST(correctness, insert_5)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    container::iterator it = c.insert(c.begin() + 4, 5);

    expect_eq(c, {1, 2, 3, 4, 5});
    EXPECT_TRUE(it == c.begin() + 4);
}

TEST(correctness, insert_6)
{
    element::no_new_instances_guard g;
    container c;
    c.push_back(1);
    container::iterator it = c.insert(c.begin(), 2);

    expect_eq(c, {2, 1});
    EXPECT_TRUE(it == c.begin());
}

TEST(correctness, insert_7)
{
    element::no_new_instances_guard g;
    container c;
    c.push_back(1);
    container::iterator it = c.insert(c.begin() + 1, 2);

    expect_eq(c, {1, 2});
    EXPECT_TRUE(it == c.begin() + 1);
}

TEST(correctness, insert_8)
{
    element::no_new_instances_guard g;
    container c;
    container::iterator it = c.insert(c.begin(), 1);

    expect_eq(c, {1});
    EXPECT_TRUE(it == c.begin());
}

TEST(correctness, erase_1)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    container::iterator it = c.erase(c.begin());

    expect_eq(c, {2, 3, 4});
    EXPECT_TRUE(it == c.begin());
}

TEST(correctness, erase_2)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    container::iterator it = c.erase(c.begin() + 1);

    expect_eq(c, {1, 3, 4});
    EXPECT_TRUE(it == c.begin() + 1);
}

TEST(correctness, erase_3)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    container::iterator it = c.erase(c.begin() + 2);

    expect_eq(c, {1, 2, 4});
    EXPECT_TRUE(it == c.begin() + 2);
}

TEST(correctness, erase_4)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    container::iterator it = c.erase(c.begin() + 3);

    expect_eq(c, {1, 2, 3});
    EXPECT_TRUE(it == c.begin() + 3);
}

TEST(correctness, erase_5)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2});
    container::iterator it = c.erase(c.begin());

    expect_eq(c, {2});
    EXPECT_TRUE(it == c.begin());
}

TEST(correctness, erase_6)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2});
    container::iterator it = c.erase(c.begin() + 1);

    expect_eq(c, {1});
    EXPECT_TRUE(it == c.begin() + 1);
}

TEST(correctness, erase_7)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1});
    container::iterator it = c.erase(c.begin());

    EXPECT_TRUE(c.empty());
    EXPECT_TRUE(it == c.begin());
}

TEST(correctness, erase_range_1)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    container::iterator it = c.erase(c.begin(), c.begin() + 2);

    expect_eq(c, {3, 4});
    EXPECT_TRUE(it == c.begin());
}

TEST(correctness, erase_range_2)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    container::iterator it = c.erase(c.begin() + 1, c.begin() + 3);

    expect_eq(c, {1, 4});
    EXPECT_TRUE(it == c.begin() + 1);
}

TEST(correctness, erase_range_3)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    container::iterator it = c.erase(c.begin() + 2, c.end());

    expect_eq(c, {1, 2});
    EXPECT_TRUE(it == c.begin() + 2);
}

TEST(correctness, erase_range_4)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});
    container::iterator it = c.erase(c.begin(), c.end());

    EXPECT_TRUE(c.empty());
    EXPECT_TRUE(it == c.begin());
}

TEST(correctness, erase_range_5)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2, 3, 4});

    c.erase(c.begin(), c.begin());
    expect_eq(c, {1, 2, 3, 4});

    c.erase(c.begin() + 1, c.begin() + 1);
    expect_eq(c, {1, 2, 3, 4});

    c.erase(c.begin() + 2, c.begin() + 2);
    expect_eq(c, {1, 2, 3, 4});

    c.erase(c.begin() + 3, c.begin() + 3);
    expect_eq(c, {1, 2, 3, 4});

    c.erase(c.begin() + 4, c.begin() + 4);
    expect_eq(c, {1, 2, 3, 4});
}

TEST(correctness, erase_range_6)
{
    element::no_new_instances_guard g;
    container c;
    mass_push_back(c, {1, 2});
    container::iterator it = c.erase(c.begin(), c.end());

    EXPECT_TRUE(c.empty());
    EXPECT_TRUE(it == c.begin());
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
