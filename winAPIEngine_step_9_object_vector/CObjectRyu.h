#pragma once

#include "SVector2D.h"

class CObjectRyu
{
public:
	CObjectRyu();
	~CObjectRyu();

	CObjectRyu(const CObjectRyu& t);
	void operator=(const CObjectRyu& t);

	inline void SetPosition(SVector2D tPosition)
	{
		mPosition = tPosition;
	}
	inline SVector2D GetPosition()
	{
		return mPosition;
	}

public:
	//��ġ ���
	SVector2D mPosition;
};
