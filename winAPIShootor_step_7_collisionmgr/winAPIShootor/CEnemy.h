#pragma once

#include "CUnit.h"
#include <vector>

#include <windows.h>

#include "CCollider.h"
using namespace std;

class CBullet;

class CEnemy: public CUnit
{
public:
	CEnemy();
	virtual ~CEnemy();

	CEnemy(const CEnemy& t);
	void operator=(const CEnemy& t);

public:
	//CEnemy타입의 객체를 복제하는 함수
	//		복사 생성자를 이용하여 객체를 생성!!!!!!
	virtual CObjectRyu* clone() override
	{
		return new CEnemy(*this);
	}

public:

	virtual void Update(float tDeltaTime) override;

	//주인공 기체는 일반탄환을 발사하는 기능을 가진다
	void DoFire(vector<CBullet*>& tBullets);	//일반탄환을 발사하는 기능
	void DoFireAimed(vector<CBullet*>& tBullets, CUnit* tpUnit);	//조준탄환을 발사하는 기능
	void DoFireCircled(vector<CBullet*>& tBullets);	//원형탄환을 발사하는 기능

	virtual void OnEnterCollision(CCollider* tOther) override
	{
		OutputDebugString(L"CEnemy::OnEnterCollision\n");

		//충돌처리

		//적 비활성화
		this->SetIsActive(false);

		tOther->GetOwnerObject()->SetIsActive(false);
	};

private:
	//임의의 탄환 이란 개념
	int mCurIndexBullet = 0;

	//타이머를 위한 변수
public:
	float mTimeTick = 0.0f;	//deltatime누적용 변수


	virtual void OnTimer() override;
};

