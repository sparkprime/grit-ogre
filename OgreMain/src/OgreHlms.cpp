/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "OgreHlms.h"

#include <algorithm>
//#include "OgreMovableObject.h"
//#include "OgreRenderable.h"
#include <fstream>

namespace Ogre
{
    const IdString HlmsPropertySkeleton             = IdString( "hlms_skeleton" );
    const IdString HlmsPropertyBonesPerVertex       = IdString( "hlms_bones_per_vertex" );

    const IdString HlmsPropertyDualParaboloidMapping= IdString( "hlms_dual_paraboloid_mapping" );

    const IdString HlmsPropertyNormal               = IdString( "hlms_normal" );

    const IdString HlmsPropertyUvCount              = IdString( "hlms_uv_count" );
    const IdString HlmsPropertyUvCount0             = IdString( "hlms_uv_count0" );
    const IdString HlmsPropertyUvCount1             = IdString( "hlms_uv_count1" );
    const IdString HlmsPropertyUvCount2             = IdString( "hlms_uv_count2" );
    const IdString HlmsPropertyUvCount3             = IdString( "hlms_uv_count3" );
    const IdString HlmsPropertyUvCount4             = IdString( "hlms_uv_count4" );
    const IdString HlmsPropertyUvCount5             = IdString( "hlms_uv_count5" );
    const IdString HlmsPropertyUvCount6             = IdString( "hlms_uv_count6" );
    const IdString HlmsPropertyUvCount7             = IdString( "hlms_uv_count7" );

    const IdString HlmsPropertyLightsDirectionalShadow  = IdString( "hlms_lights_directional_shadow" );
    const IdString HlmsPropertyLightsPointShadow        = IdString( "hlms_lights_point_shadow" );
    const IdString HlmsPropertyLightsSpotShadow         = IdString( "hlms_lights_spot_shadow" );

    const IdString HlmsPropertyShadowCastingLights  = IdString( "hlms_shadow_casting_lights" );
    const IdString HlmsPropertyPssmSplits           = IdString( "hlms_pssm_splits" );

    const IdString PropertyDiffuseMap   = IdString( "diffuse_map" );
    const IdString PropertyNormalMap    = IdString( "normal_map" );
    const IdString PropertySpecularMap  = IdString( "specular_map" );
    const IdString PropertyEnvProbeMap  = IdString( "envprobe_map" );

    Hlms::Hlms()
    {
    }
    //-----------------------------------------------------------------------------------
    Hlms::~Hlms()
    {
    }
    //-----------------------------------------------------------------------------------
    void Hlms::setCommonProperties(void)
    {
        uint16 numWorldTransforms = 2;
        //bool castShadows          = true;

        setProperty( HlmsPropertySkeleton, numWorldTransforms > 1 );
        setProperty( HlmsPropertyUvCount, 2 );
        setProperty( "true", 1 );
        setProperty( "false", 0 );

        setProperty( HlmsPropertyDualParaboloidMapping, 0 );

        setProperty( HlmsPropertyNormal, 1 );

        setProperty( HlmsPropertyUvCount0, 2 );
        setProperty( HlmsPropertyUvCount1, 4 );
        setProperty( HlmsPropertyBonesPerVertex, 4 );

        setProperty( HlmsPropertyShadowCastingLights, 3 );
        setProperty( HlmsPropertyPssmSplits, 3 );

        setProperty( HlmsPropertyLightsDirectionalShadow, 1 );
        setProperty( HlmsPropertyLightsPointShadow, 2 );
        setProperty( HlmsPropertyLightsSpotShadow, 3 );

        setProperty( PropertyDiffuseMap, 1 );
        setProperty( PropertyNormalMap, 1 );
        setProperty( PropertySpecularMap, 1 );
        setProperty( PropertyEnvProbeMap, 1 );
    }
    //-----------------------------------------------------------------------------------
    void Hlms::setProperty( IdString key, int32 value )
    {
        Property p( key, value );
        PropertyVec::iterator it = std::lower_bound( mSetProperties.begin(), mSetProperties.end(),
                                                     p, OrderPropertyByIdString );
        if( it == mSetProperties.end() || it->keyName != p.keyName )
            mSetProperties.insert( it, p );
        else
            *it = p;
    }
    //-----------------------------------------------------------------------------------
    int32 Hlms::getProperty(IdString key, int32 defaultVal ) const
    {
        Property p( key, 0 );
        PropertyVec::const_iterator it = std::lower_bound( mSetProperties.begin(), mSetProperties.end(),
                                                           p, OrderPropertyByIdString );
        if( it != mSetProperties.end() && it->keyName == p.keyName )
            defaultVal = it->value;

        return defaultVal;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::findBlockEnd( SubStringRef &outSubString, bool &syntaxError )
    {
        char *blockNames[] =
        {
            "foreach",
            "property",
            "piece"
        };

        String::const_iterator it = outSubString.begin();
        String::const_iterator en = outSubString.end();

        int nesting = 0;

        while( it != en && nesting >= 0 )
        {
            if( *it == '@' )
            {
                SubStringRef subString( &outSubString.getOriginalBuffer(), it + 1 );

                size_t idx = subString.find( "end" );
                if( idx == 0 )
                {
                    --nesting;
                    it += sizeof( "end" ) - 1;
                }
                else
                {
                    for( size_t i=0; i<sizeof( blockNames ) / sizeof( char* ); ++i )
                    {
                        size_t idx = subString.find( blockNames[i] );
                        if( idx == 0 )
                        {
                            it = subString.begin() + strlen( blockNames[i] );
                            ++nesting;
                            break;
                        }
                    }
                }
            }

            ++it;
        }

        assert( nesting >= -1 );

        if( it != en && nesting < 0 )
            outSubString.setEnd( it - outSubString.getOriginalBuffer().begin() - sizeof( "end" ) );
        else
        {
            syntaxError = false;
            printf( "Syntax Error at line %i: start block (e.g. @foreach; @property) "
                    "without matching @end\n", calculateLineCount( outSubString ) );
        }
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::evaluateExpression( SubStringRef &outSubString, bool &outSyntaxError ) const
    {
        size_t expEnd = evaluateExpressionEnd( outSubString );

        if( expEnd == String::npos )
        {
            outSyntaxError = true;
            return false;
        }

        SubStringRef subString( &outSubString.getOriginalBuffer(), outSubString.getStart(),
								 outSubString.getStart() + expEnd );

		outSubString = SubStringRef( &outSubString.getOriginalBuffer(),
                                     outSubString.getStart() + expEnd + 1 );

        bool textStarted = false;
        bool syntaxError = false;
        bool nextExpressionNegates = false;

        std::vector<Expression*> expressionParents;
        ExpressionVec outExpressions;
        outExpressions.clear();
        outExpressions.resize( 1 );

        Expression *currentExpression = &outExpressions.back();

        String::const_iterator it = subString.begin();
        String::const_iterator en = subString.end();

        while( it != en && !syntaxError )
        {
            char c = *it;

            if( c == '(' )
            {
                currentExpression->children.push_back( Expression() );
                expressionParents.push_back( currentExpression );

                currentExpression->children.back().negated = nextExpressionNegates;

                textStarted = false;
                nextExpressionNegates = false;

                currentExpression = &currentExpression->children.back();
            }
            else if( c == ')' )
            {
				if( expressionParents.empty() )
					syntaxError = true;
				else
				{
					currentExpression = expressionParents.back();
					expressionParents.pop_back();
				}

                textStarted = false;
            }
            else if( c == ' ' || c == '\t' || c == '\n' || c == '\r' )
            {
                textStarted = false;
            }
            else if( c == '!' )
            {
                nextExpressionNegates = true;
            }
            else
            {
                if( !textStarted )
                {
                    textStarted = true;
                    currentExpression->children.push_back( Expression() );
                    currentExpression->children.back().negated = nextExpressionNegates;
                }

                if( c == '&' || c == '|' )
                {
                    if( currentExpression->children.empty() || nextExpressionNegates )
                    {
                        syntaxError = true;
                    }
                    else if( !currentExpression->children.back().value.empty() &&
                             c != currentExpression->children.back().value.back() )
                    {
                        currentExpression->children.push_back( Expression() );
                    }
                }

                currentExpression->children.back().value.push_back( c );
                nextExpressionNegates = false;
            }

            ++it;
        }

        bool retVal = false;

        if( !expressionParents.empty() )
            syntaxError = true;

        if( !syntaxError )
            retVal = evaluateExpressionRecursive( outExpressions, syntaxError );

        if( syntaxError )
            printf( "Syntax Error at line %i\n", calculateLineCount( subString ) );

        outSyntaxError = syntaxError;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::evaluateExpressionRecursive( ExpressionVec &expression, bool &outSyntaxError ) const
    {
        ExpressionVec::iterator itor = expression.begin();
        ExpressionVec::iterator end  = expression.end();

        while( itor != end )
        {
            if( itor->value == "&&" )
                itor->type = EXPR_OPERATOR_AND;
            else if( itor->value == "||" )
                itor->type = EXPR_OPERATOR_OR;
            else if( !itor->children.empty() )
                itor->type = EXPR_OBJECT;
            else
                itor->type = EXPR_VAR;

            ++itor;
        }

        bool syntaxError = outSyntaxError;
        bool lastExpWasOperator = true;

        itor = expression.begin();

        while( itor != end && !syntaxError )
        {
            Expression &exp = *itor;
            if( ((exp.type == EXPR_OPERATOR_OR || exp.type == EXPR_OPERATOR_AND) && lastExpWasOperator) ||
                ((exp.type == EXPR_VAR || exp.type == EXPR_OBJECT) && !lastExpWasOperator ) )
            {
                syntaxError = true;
            }
            else if( exp.type == EXPR_OPERATOR_OR || exp.type == EXPR_OPERATOR_AND )
            {
                lastExpWasOperator = true;
            }
            else if( exp.type == EXPR_VAR )
            {
                exp.result = getProperty( exp.value ) != 0;
                lastExpWasOperator = false;
            }
            else
            {
                exp.result = evaluateExpressionRecursive( exp.children, syntaxError );
                lastExpWasOperator = false;
            }

            ++itor;
        }

        bool retVal = true;

        if( !syntaxError )
        {
            itor = expression.begin();
            bool andMode = true;

            while( itor != end )
            {
                if( itor->type == EXPR_OPERATOR_OR )
                    andMode = false;
                else if( itor->type == EXPR_OPERATOR_AND )
                    andMode = true;
                else
                {
                    if( andMode )
                        retVal &= itor->negated ? !itor->result : itor->result;
                    else
                        retVal |= itor->negated ? !itor->result : itor->result;
                }

                ++itor;
            }
        }

        outSyntaxError = syntaxError;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    size_t Hlms::evaluateExpressionEnd( const SubStringRef &outSubString )
    {
        String::const_iterator it = outSubString.begin();
        String::const_iterator en = outSubString.end();

        int nesting = 0;

        while( it != en && nesting >= 0 )
        {
            if( *it == '(' )
                ++nesting;
            else if( *it == ')' )
                --nesting;
            ++it;
        }

        assert( nesting >= -1 );

        size_t retVal = String::npos;
        if( it != en && nesting < 0 )
        {
            retVal = it - outSubString.begin() - 1;
        }
        else
        {
            printf( "Syntax Error at line %i: opening parenthesis without matching closure\n",
                    calculateLineCount( outSubString ) );
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::evaluateParamArgs( SubStringRef &outSubString, StringVec &outArgs, bool &outSyntaxError )
    {
        size_t expEnd = evaluateExpressionEnd( outSubString );

        if( expEnd == String::npos )
        {
            outSyntaxError = true;
            return;
        }

        SubStringRef subString( &outSubString.getOriginalBuffer(), outSubString.getStart(),
                                 outSubString.getStart() + expEnd );

        outSubString = SubStringRef( &outSubString.getOriginalBuffer(),
                                     outSubString.getStart() + expEnd + 1 );

        int expressionState = 0;
        bool syntaxError = false;

        outArgs.clear();
        outArgs.push_back( String() );

        String::const_iterator it = subString.begin();
        String::const_iterator en = subString.end();

        while( it != en && !syntaxError )
        {
            char c = *it;

            if( c == '(' || c == ')' || c == '@' || c == '&' || c == '|' )
            {
                syntaxError = true;
            }
            else if( c == ' ' || c == '\t' || c == '\n' || c == '\r' )
            {
                if( expressionState == 1 )
                    expressionState = 2;
            }
            else if( c == ',' )
            {
                expressionState = 0;
                outArgs.push_back( String() );
            }
            else
            {
                if( expressionState == 2 )
                {
                    printf( "Syntax Error at line %i: ',' or ')' expected\n",
                            calculateLineCount( subString ) );
                    syntaxError = true;
                }
                else
                {
                    outArgs.back().push_back( *it );
                    expressionState = 1;
                }
            }

            ++it;
        }

        if( syntaxError )
            printf( "Syntax Error at line %i\n", calculateLineCount( subString ) );

        outSyntaxError = syntaxError;
    }
    //-----------------------------------------------------------------------------------
    void Hlms::copy( String &outBuffer, const SubStringRef &inSubString, size_t length )
    {
        String::const_iterator itor = inSubString.begin();
        String::const_iterator end  = inSubString.begin() + length;

        while( itor != end )
            outBuffer.push_back( *itor++ );
    }
    //-----------------------------------------------------------------------------------
    void Hlms::repeat( String &outBuffer, const SubStringRef &inSubString, size_t length,
                       size_t passNum, const String &counterVar )
    {
        String::const_iterator itor = inSubString.begin();
        String::const_iterator end  = inSubString.begin() + length;

        while( itor != end )
        {
            if( *itor == '@' && !counterVar.empty() )
            {
                SubStringRef subString( &inSubString.getOriginalBuffer(), itor + 1 );
                if( subString.find( counterVar ) == 0 )
                {
                    char tmp[16];
                    sprintf( tmp, "%i", passNum );
                    outBuffer += tmp;
                    itor += counterVar.size() + 1;
                }
                else
                {
                    outBuffer.push_back( *itor++ );
                }
            }
            else
            {
               outBuffer.push_back( *itor++ );
            }
        }
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::parseForEach( const String &inBuffer, String &outBuffer ) const
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVec argValues;
        SubStringRef subString( &inBuffer, 0 );
        size_t pos = subString.find( "@foreach" );

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + sizeof( "@foreach" ) );
            evaluateParamArgs( subString, argValues, syntaxError );

            SubStringRef blockSubString = subString;
            findBlockEnd( blockSubString, syntaxError );

            if( !syntaxError )
            {
                char *endPtr;
                int count = strtol( argValues[0].c_str(), &endPtr, 10 );
                if( argValues[0].c_str() == endPtr )
                {
                    //This isn't a number. Let's try if it's a variable
                    count = getProperty( argValues[0], -1 );
                }

                if( count < 0 )
                {
                    printf( "Invalid parameter at line %i (@foreach)."
                            " '%s' is not a number nor a variable\n",
                            calculateLineCount( blockSubString ), argValues[0].c_str() );
                    syntaxError = true;
                    count = 0;
                }

                String counterVar;
                if( argValues.size() > 1 )
                    counterVar = argValues[1];

                int start = 0;
                if( argValues.size() > 2 )
                {
                    start = strtol( argValues[2].c_str(), &endPtr, 10 );
                    if( argValues[2].c_str() == endPtr )
                    {
                        //This isn't a number. Let's try if it's a variable
                        start = getProperty( argValues[2], -1 );
                    }

                    if( start < 0 )
                    {
                        printf( "Invalid parameter at line %i (@foreach)."
                                " '%s' is not a number nor a variable\n",
                                calculateLineCount( blockSubString ), argValues[2].c_str() );
                        syntaxError = true;
                        start = 0;
                        count = 0;
                    }
                }

                for( int i=start; i<count; ++i )
                    repeat( outBuffer, blockSubString, blockSubString.getSize(), i, counterVar );

            }

            subString.setStart( blockSubString.getEnd() + sizeof( "@end" ) );
            pos = subString.find( "@foreach" );
        }

        copy( outBuffer, subString, subString.getSize() );

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::parseProperties( String &inBuffer, String &outBuffer ) const
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        SubStringRef subString( &inBuffer, 0 );
        size_t pos = subString.find( "@property" );

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + sizeof( "@property" ) );
            bool result = evaluateExpression( subString, syntaxError );

            SubStringRef blockSubString = subString;
            findBlockEnd( blockSubString, syntaxError );

            if( result && !syntaxError )
                copy( outBuffer, blockSubString, blockSubString.getSize() );

            subString.setStart( blockSubString.getEnd() + sizeof( "@end" ) );
            pos = subString.find( "@property" );
        }

        copy( outBuffer, subString, subString.getSize() );

        while( !syntaxError && outBuffer.find( "@property" ) != String::npos )
        {
            inBuffer.swap( outBuffer );
            syntaxError = parseProperties( inBuffer, outBuffer );
        }

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::collectPieces( const String &inBuffer, String &outBuffer )
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVec argValues;
        SubStringRef subString( &inBuffer, 0 );
        size_t pos = subString.find( "@piece" );

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + sizeof( "@piece" ) );
            evaluateParamArgs( subString, argValues, syntaxError );

            syntaxError |= argValues.size() != 1;

            if( !syntaxError )
            {
                const IdString pieceName( argValues[0] );
                PiecesMap::const_iterator it = mPieces.find( pieceName );
                if( it != mPieces.end() )
                {
                    syntaxError = true;
                    printf( "Error at line %i: @piece '%s' already defined",
                            calculateLineCount( subString ), argValues[0].c_str() );
                }
                else
                {
                    SubStringRef blockSubString = subString;
                    findBlockEnd( blockSubString, syntaxError );

                    String tmpBuffer;
                    copy( tmpBuffer, blockSubString, blockSubString.getSize() );
                    mPieces[pieceName] = tmpBuffer;

                    subString.setStart( blockSubString.getEnd() + sizeof( "@end" ) );
                }
            }
            else
            {
                printf( "Syntax Error at line %i: @piece expects one parameter",
                        calculateLineCount( subString ) );
            }

            pos = subString.find( "@piece" );
        }

        copy( outBuffer, subString, subString.getSize() );

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::insertPieces( String &inBuffer, String &outBuffer ) const
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVec argValues;
        SubStringRef subString( &inBuffer, 0 );
        size_t pos = subString.find( "@insertpiece" );

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos + sizeof( "@insertpiece" ) );
            evaluateParamArgs( subString, argValues, syntaxError );

            syntaxError |= argValues.size() != 1;

            if( !syntaxError )
            {
                const IdString pieceName( argValues[0] );
                PiecesMap::const_iterator it = mPieces.find( pieceName );
                if( it != mPieces.end() )
                    outBuffer += it->second;
            }
            else
            {
                printf( "Syntax Error at line %i: @insertpiece expects one parameter",
                        calculateLineCount( subString ) );
            }

            pos = subString.find( "@insertpiece" );
        }

        copy( outBuffer, subString, subString.getSize() );

        while( !syntaxError && outBuffer.find( "@insertpiece" ) != String::npos )
        {
            inBuffer.swap( outBuffer );
            syntaxError = insertPieces( inBuffer, outBuffer );
        }

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::parseCounter( const String &inBuffer, String &outBuffer )
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        StringVec argValues;
        SubStringRef subString( &inBuffer, 0 );
        size_t _pos[2];
        _pos[0] = subString.find( "@counter" );
        _pos[1] = subString.find( "@value" );
        size_t pos;

        int keyword = 0;
        if( _pos[0] <= _pos[1] )
        {
            keyword = 0;
            pos = _pos[0];
        }
        else
        {
            keyword = 1;
            pos = _pos[1];
        }

        bool syntaxError = false;

        while( pos != String::npos && !syntaxError )
        {
            //Copy what comes before the block
            copy( outBuffer, subString, pos );

            subString.setStart( subString.getStart() + pos +
                                (keyword == 0 ? sizeof( "@counter" ) : sizeof( "@value" )) );
            evaluateParamArgs( subString, argValues, syntaxError );

            syntaxError |= argValues.size() != 1;

            if( !syntaxError )
            {
                const IdString propertyKey( argValues[0] );
                int32 count = getProperty( propertyKey );
                char tmp[16];
                sprintf( tmp, "%i", count );
                outBuffer += tmp;

                if( keyword == 0 )
                {
                    ++count;
                    setProperty( propertyKey, count );
                }
            }
            else
            {
                printf( "Syntax Error at line %i: @counter/@value expect one parameter",
                        calculateLineCount( subString ) );
            }

            _pos[0] = subString.find( "@counter" );
            _pos[1] = subString.find( "@value" );
            if( _pos[0] <= _pos[1] )
            {
                keyword = 0;
                pos = _pos[0];
            }
            else
            {
                keyword = 1;
                pos = _pos[1];
            }
        }

        copy( outBuffer, subString, subString.getSize() );

        return syntaxError;
    }
    //-----------------------------------------------------------------------------------
    bool Hlms::parse( const String &inBuffer, String &outBuffer ) const
    {
        outBuffer.clear();
        outBuffer.reserve( inBuffer.size() );

        return parseForEach( inBuffer, outBuffer );
        //return parseProperties( inBuffer, outBuffer );
    }
    //-----------------------------------------------------------------------------------
    /*void Hlms::processParams( HlmsParams &inOutParams )
    {
        uint32 hash[2];
        hash[0] = getProperty( HlmsPropertySkeleton ) |
                (getProperty( HlmsPropertyDualParaboloidMapping ) << 1) |
                (getProperty( HlmsPropertyUvCount ) << 2) |
                ((getProperty( HlmsPropertyUvCount0 ) - 1) << 6 )|
                ((getProperty( HlmsPropertyUvCount1 ) - 1) << 8 )|
                ((getProperty( HlmsPropertyUvCount2 ) - 1) << 10)|
                ((getProperty( HlmsPropertyUvCount3 ) - 1) << 12)|
                ((getProperty( HlmsPropertyUvCount4 ) - 1) << 14)|
                ((getProperty( HlmsPropertyUvCount5 ) - 1) << 16)|
                ((getProperty( HlmsPropertyUvCount6 ) - 1) << 18)|
                ((getProperty( HlmsPropertyUvCount7 ) - 1) << 20)|
                (getProperty( HlmsPropertyShadowCastingLights ) << 22)|
                (getProperty( HlmsPropertyPssmSplits ) << 26);
        hash[1] = getProperty( HlmsPropertyLightsDirectionalShadow )|
                (getProperty( HlmsPropertyLightsPointShadow ) << 4 )|
                (getProperty( HlmsPropertyLightsSpotShadow )  << 8 )|
                (getProperty( PropertyDiffuseMap )  << 12 ) |;
                (getProperty( PropertyNormalMap )   << 13 ) |;
                (getProperty( PropertySpecularMap ) << 14 ) |;
                (getProperty( PropertyEnvProbeMap ) << 15 );

        uint32 retVal = 0;
        MurmurHash3_x86_32( hash, sizeof( hash ), IdString::Seed, &retVal );
        return retVal;
    }*/
    //-----------------------------------------------------------------------------------
    /*MaterialPtr Hlms::generateFor( Renderable *renderable, MovableObject *movableObject )
    {
        uint16 numWorldTransforms   = renderable->getNumWorldTransforms();//TODO
        bool castShadows            = movableObject->getCastShadows();
    }*/
    void Hlms::generateFor()
    {
        uint16 numWorldTransforms = 1;
        bool castShadows          = true;

        /*std::ifstream inFile( "E:/Projects/Hlms/bin/Hlms/PBS/GLSL/VertexShader_vs.glsl",
                              std::ios::in | std::ios::binary );
        std::ofstream outFile( "E:/Projects/Hlms/bin/Hlms/PBS/GLSL/Output_vs.glsl",
                               std::ios::out | std::ios::binary );*/
        std::ifstream inFile( "E:/Projects/Hlms/bin/Hlms/PBS/GLSL/PixelShader_ps.glsl",
                                      std::ios::in | std::ios::binary );
        std::ofstream outFile( "E:/Projects/Hlms/bin/Hlms/PBS/GLSL/Output_ps.glsl",
                               std::ios::out | std::ios::binary );

        String inString;
        String outString;

        inFile.seekg( 0, std::ios::end );
		inString.resize( inFile.tellg() );
        inFile.seekg( 0, std::ios::beg );

        inFile.read( &inString[0], inString.size() );

        setCommonProperties();
        //this->parse( inString, outString );
        this->parseForEach( inString, outString );
        this->parseProperties( outString, inString );
        this->collectPieces( inString, outString );
        this->insertPieces( outString, inString );
        this->parseCounter( inString, outString );

        outFile.write( &outString[0], outString.size() );
    }
    //-----------------------------------------------------------------------------------
    size_t Hlms::calculateLineCount( const String &buffer, size_t idx )
    {
        String::const_iterator itor = buffer.begin();
        String::const_iterator end  = buffer.begin() + idx;

        size_t lineCount = 0;

        while( itor != end )
        {
            if( *itor == '\n' )
                ++lineCount;
            ++itor;
        }

        return lineCount + 1;
    }
    //-----------------------------------------------------------------------------------
    size_t Hlms::calculateLineCount( const SubStringRef &subString )
    {
        return calculateLineCount( subString.getOriginalBuffer(), subString.getStart() );
    }
}
