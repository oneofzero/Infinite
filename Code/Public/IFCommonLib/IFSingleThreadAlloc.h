

class IFSingleThreadAlloc
{
public:
	void* alloc(int size);

	void free(void*);

private:

	struct MemNode
	{
		int left;
		int right;
		int allocSize;
		int blockSize;
	};

	MemNode* getNode(int offset)
	{
		return nullptr;
	}

	int m_nFreeRoot;
};