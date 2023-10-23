#pragma once
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

namespace chk
{
	enum class PieceType
	{
		Red = 69995,
		Black = 78885,
	};

	constexpr auto BLACK_NORMAL = "resources/normal_black.png";
	constexpr auto BLACK_KING = "resources/king_black.png";
	constexpr auto RED_NORMAL = "resources/normal_red.png";
	constexpr auto RED_KING = "resources/king_red.png";

	class Piece final : public sf::Drawable, public sf::Transformable
	{

	public:
		Piece(const sf::CircleShape &circle, const PieceType _pType);
		PieceType getPieceType() const;
		void activateKing();
		bool getIsKing() const;
		void moveCustom(float posX, float posY);

	private:
		sf::Texture texture;
		sf::CircleShape myCircle;
		PieceType pieceType;
		bool isKing = false;
		void Piece::draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	};

	inline Piece::Piece(const sf::CircleShape &circle, const PieceType _pType)
	{
		this->myCircle = circle;
		this->pieceType = _pType;
		this->isKing = false;

		sf::Texture localTxr;
		if (pieceType == PieceType::Red)
		{
			/* code */
			if (localTxr.loadFromFile(RED_NORMAL))
			{
				this->texture = localTxr;
				this->myCircle.setTexture(&this->texture);
			}
		}
		else
		{
			if (localTxr.loadFromFile(BLACK_NORMAL))
			{
				this->texture = localTxr;
				this->myCircle.setTexture(&this->texture);
			}
		}
	}

	inline void Piece::draw(sf::RenderTarget &target, sf::RenderStates states) const
	{
		target.draw(this->myCircle, states);
	}

	/**
	 * get piece type, whether black or red
	 */
	inline PieceType Piece::getPieceType() const
	{
		return this->pieceType;
	}

	/**
	 * set piece as King. Will also change Piece Texture
	 */
	inline void Piece::activateKing()
	{
		this->isKing = true;
		sf::Texture localTxr;
		if (pieceType == PieceType::Red)
		{
			/* code */
			if (localTxr.loadFromFile(RED_KING))
			{
				this->texture = localTxr;
				this->myCircle.setTexture(&this->texture);
			}
		}
		else
		{
			if (localTxr.loadFromFile(BLACK_KING))
			{
				this->texture = localTxr;
				this->myCircle.setTexture(&this->texture);
			}
		}
	}

	/**
	 * get whether dir is king
	 */
	inline bool Piece::getIsKing() const
	{
		return this->isKing;
	}

	/**
	 * move piece across board to global position (x,y)
	 * @param posX by x position
	 * @param posY the y position
	 */
	inline void Piece::moveCustom(const float posX, const float posY)
	{
		//TODO Validate move, and verify if is King. 
		this->setPosition(sf::Vector2f(posX, posY));
	}

}