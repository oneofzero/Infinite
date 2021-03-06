// alloc.cpp: 定义控制台应用程序的入口点。
//
#include "test.h"
#include <vector>
#include <assert.h>
#include "lua.hpp"

struct MemNode
{
	int m_size;
	
	int m_childB;
	int m_parent;
	bool m_full;
	bool m_haveChildA;
	//bool m_free;
	MemNode* childA()
	{
		if(m_haveChildA)
			return (MemNode*)((char*)this+sizeof(MemNode));
		return NULL;
	}
	MemNode* childB()
	{
		if(m_childB)
			return (MemNode*)((char*)this + m_childB);
		return NULL;
	}
	MemNode* parent()
	{
		if(m_parent)
			return  (MemNode*)((char*)this + m_parent);
		return NULL;
	}

	int memSize()
	{
		return m_size - sizeof(MemNode);
	}

	char* mem()
	{
		return ((char*)this + sizeof(MemNode));
	}

	bool isLeaf()
	{
		return childA()==NULL&&childB()==NULL;
	}

	bool isFull()
	{
		return m_full;
	}

	bool checkfull()
	{
		if (isLeaf())
			return m_full;
		else
		{
			m_full = childA()->isFull() && (m_childB ? childB()->isFull() : true);
			if (m_full && parent())
				parent()->checkfull();
		}
	}

	void free(char* p)
	{
		MemNode* pChildA = (MemNode*)(p - sizeof(MemNode));

	}

	void free()
	{
		MemNode* node = this;
		while (node)
		{
			node->m_full = false;
			MemNode* parentNode = node->parent();
			if (node->isLeaf())
			{

				if (parentNode)
				{
					MemNode* pOther = NULL;
					if (node == parentNode->childA())
					{
						pOther = parentNode->childB();

						if (!pOther)
							parentNode->m_haveChildA = false;
					}
					else
					{
						pOther = parentNode->childA();

					}
					if (pOther && pOther->isLeaf() && pOther->m_full == false)
					{
						//delete pOther;
						//m_pNodePool->Free( pOther );
						parentNode->m_childB = 0;
						//delete pNode;
						//m_pNodePool->Free(pNode);
						parentNode->m_haveChildA = 0;
					}





				}
			}

			node = parentNode;
		}

	}

	MemNode* alloc(int size)
	{
		//size = (size / 4 + 1) * 4;

		if (m_full || memSize() <size)
			return NULL;


		MemNode* node = NULL;
		if (childA())
		{
			node = childA()->alloc(size);
		}
		if (!node && childB())
		{
			node = childB()->alloc(size);
			if (!node)
				return NULL;
		}
		if (!node)
		{
			if (!isLeaf())
				return NULL;

			if (size + sizeof(MemNode) > memSize())
				return NULL;

			int halfsize = memSize() / 4 - sizeof(MemNode)*2;

			if (halfsize < size)
			{
				node = this;
				node->m_full = true;
				return this;
			}

			m_haveChildA = true;
			childA()->m_full = false;
			childA()->m_haveChildA = false;
			childA()->m_childB = 0;
			childA()->m_parent = (char*)this - (char*)childA();
			assert(childA()->m_parent < 0);
			childA()->m_size = memSize() / 4;

			m_childB = m_size - (3*memSize() / 4);
			childB()->m_haveChildA = false;
			childB()->m_childB = 0;
			childB()->m_parent = (char*)this - (char*)childB();
			assert(childB()->m_parent < 0);
			childB()->m_size = 3*memSize() / 4 ;
			childB()->m_full = false;
			
			return childA()->alloc(size);
				
		}
		if (childA() && childA()->isFull() && childB() && childB()->isFull())
		{
			m_full = true;
		}

		return node;

	}

	int getFreeSize(int* maxblook = NULL)
	{
		if (isLeaf())
		{
			if (m_full)
				return 0;
			if (maxblook)
			{
				*maxblook = *maxblook < memSize() ? memSize() : *maxblook;
			}
			return memSize();
		}
		else
		{
			int size = 0;
			if (childA())
				size = childA()->getFreeSize(maxblook);
			if (childB())
				size += childB()->getFreeSize(maxblook);
			
			return size;
		}
	}
};



MemNode* g_mem_root = NULL;
void mem_init(char* buf, int size)
{
	g_mem_root = (MemNode*)buf;
	g_mem_root->m_size = size;
	g_mem_root->m_parent = 0;
	g_mem_root->m_haveChildA = false;
	g_mem_root->m_childB = 0;
	g_mem_root->m_full = false;
}

char* mem_alloc(int size)
{
	if (size == 0)
		return NULL;
	auto node = g_mem_root->alloc(size);
	if(!node)
		return NULL;
	auto p = node->mem();

	return p;
}

void mem_free(char* p)
{
	if (p == NULL)
		return;
	auto node = (MemNode*)(p - sizeof(MemNode));
	node->free();
}

char* mem_realloc(char* p, int newsize)
{
	if (p)
	{
		auto node = (MemNode*)(p - sizeof(MemNode));
		if (node->memSize() >= newsize)
			return node->mem();
		if (newsize)
		{
			char* newmem = mem_alloc(newsize);
			if (newmem)
			{
				memcpy(newmem, node->mem(), node->memSize());
				node->free();
			}
			else
			{
				printf("cant alloc%d\r\n", newsize);
			}
			return newmem;
		}
		else
			node->free();
		return NULL;
		
	}
	else
		return mem_alloc(newsize);

}
int g_allocIdx = 0;
void * (LUAAlloc)(void *ud, void *ptr, size_t osize, size_t nsize)
{
	g_allocIdx++;
	if (nsize == 0) {
		mem_free((char*)ptr);
		return NULL;
	}
	else
	return mem_realloc((char*)ptr, nsize);
}
char alloc_buf[1024 * 1024 * 6];


int maintest()
{
	int nodesize = sizeof(MemNode);
	printf("node size %d \r\n", nodesize);

	mem_init(alloc_buf, sizeof(alloc_buf));
	auto root = g_mem_root;

	auto p = mem_alloc(999);
	p = mem_realloc(p, 1024);

	int allocsize = 0;
	for (int j = 0; j < 1; j++)
	{
		allocsize = 0;
		std::vector<MemNode*> nodes;
		printf("start %d alloc\r\n", j);
		for (int i = 0; i < 100000; i++)
		{
			int size = ((rand() % 200) / 4 + 1) * 4;
			auto node = root->alloc(size);
			if (!node)
			{
				printf("allock break at %d alloc:%d free:%d\r\n", i, allocsize, root->getFreeSize());
				break;
			}
			nodes.push_back(node);
			allocsize += size;
		}
		
		for (int i = 0; i < (int)nodes.size()-1; i++)
		{
			nodes[i]->free();
		}
		if(nodes.size()>0)
			nodes[nodes.size() - 1]->free();
	}

	printf("alloc %dbytes\r\n", allocsize);
	printf("free %dbytes\r\n", root->getFreeSize());
	//luaL_newstate()
	auto L = lua_newstate(LUAAlloc,0);
	luaL_openlibs(L);

	printf("free %dbytes\r\n", root->getFreeSize());
	const char* script = "local t = {} for i=1,190000 do t[i]=i end print('done!',collectgarbage('count')/1024 )";

	if (luaL_dostring(L,script) != 0)
	{
		printf("run error%s\r\n", lua_tostring(L, -1));
	}
	int maxblock = 0;
	printf("free %dbytes max:%d\r\n", root->getFreeSize(&maxblock) , maxblock);

	lua_close(L);
	printf("free %dbytes\r\n", root->getFreeSize());

    return 0;
}

