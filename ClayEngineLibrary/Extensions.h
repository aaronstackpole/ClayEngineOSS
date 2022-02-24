#pragma once
/******************************************************************************/
/*                                                                            */
/* ClayEngine Interface and Extension Library (C) 2022 Epoch Meridian, LLC.   */
/*                                                                            */
/*                                                                            */
/******************************************************************************/

namespace ClayEngine
{
	namespace Extensions
	{
		struct IUpdate
		{
			virtual void Update(float elapsedTime) = 0;
		};

		struct IDraw
		{
			virtual void Draw() = 0;
		};
	}
}
