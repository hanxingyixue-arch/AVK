#include "all.h"
#include <vector>
#include <memory>

using std::vector;

struct node;

using node_ptr = std::unique_ptr<node>;

struct node
{
	static node_ptr make(uint radix_size, bool is_leaf)
	{
		node_ptr r = node_ptr(new node{});
		r->leaf = is_leaf;
		if (r->leaf)
			r->values.resize(radix_size);
		else
			r->next.resize(radix_size);
		return r;
	}

	node()
	{
		memset(this, 0, sizeof(node));
	}

	~node()
	{
		if (leaf)
			values.~vector();
		else
			next.~vector();
	}

	bool leaf;
	union
	{
		vector<node_ptr>			next;
		vector<vector<item>>		values;
	};
};

node_ptr build_radix_tree(main_array array, uint radix_size)
{
	node_ptr root = nullptr;
	for (item& e : array)
	{
		auto n = &root;
		for (sint depth = item::max_radix(radix_size) - 1; depth > 0; --depth)
		{
			const uint radix = extract_radix(e, depth, radix_size);
			if (*n == nullptr)
				*n = node::make(radix_size, false);
			n = &(*n)->next[radix];
		}
		const uint radix = extract_radix(e, 0, radix_size);
		if (*n == nullptr)
			*n = node::make(radix_size, true);
		(*n)->values[radix].push_back(e);
	}
	return root;
}

void traverse_radix_tree(node& node, uint depth, uint max_radix, item*& out_begin)
{
	if (depth == item::max_radix(max_radix) - 1)
	{
		for (auto& bin : node.values)
		{
			if (bin.size() != 0)
			{
				for (auto& e : bin)
				{
					*out_begin = e;
					++out_begin;
				}
			}
		}
	}
	else
	{
		for (auto& next : node.next)
		{
			if (next != nullptr)
			{
				traverse_radix_tree(*next, depth + 1, max_radix, out_begin);
			}
		}
	}
}

void radix_tree_sort(main_array array)
{
	const uint default_radix_size = 256;
	auto root = build_radix_tree(array, default_radix_size);
	auto out = array.data();
	traverse_radix_tree(*root, 0, default_radix_size, out);
}