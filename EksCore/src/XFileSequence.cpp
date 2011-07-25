#include "XFileSequence"
#include "XVariant"

#include "XDebug"

XFileSequence::XFileSequence( XString in, stringMode mode )
    {
    setSequence( in, mode );
    }

XFileSequence::XFileSequence( const char *in, stringMode mode )
    {
    setSequence( XString( in ), mode );
    }

XFileSequence::XFileSequence( XVariant in )
    {
    XVariantList list( in.toList() );
    if( list.size() == 4 )
        {
        _pre = list[0].toString();
        _padding = list[1].toInt();
        _post = list[2].toString();
        _hasNumber = list[3].toBool();
        }
    else
        {
        _hasNumber = false;
        _padding = 0;
        }
    }

XFileSequence::operator XVariant()
    {
    XVariantList list;
    return list << _pre << _padding << _post << _hasNumber;
    }

void XFileSequence::setSequence( XString in, stringMode mode )
    {
    _padding = 0;
    _pre = in;
    _hasNumber = false;
    if( mode == Auto )
        {
        mode = NumericExtract;
        QRegExp rx( "\\$([0-9]+)F" );
        if( rx.indexIn( in, 0 ) >= 0 )
            {
            mode = Parsed;
            }
        }

    if( mode == Parsed )
        {
        QRegExp rx( "\\$([0-9]+)F" );

        int pos( 0 );
        int start( -1 );
        uint length( 0 );
        QString capturedPadding;

        while( pos >= 0 )
            {
             pos = rx.indexIn( in, pos );
             if( pos >= 0 )
                {
                capturedPadding = rx.cap(1);
                start = pos;
                length = rx.matchedLength();
                pos += rx.matchedLength();
                }
            }

        if( start >= 0 )
            {
            _hasNumber = true;
            _pre = in.mid( 0, start );
            _padding = capturedPadding.toInt();
            _post = in.mid( start+rx.cap(1).length()+3 );
            }
        }
    else
        {
        QRegExp rx( "([0-9]+)" );

        int pos( 0 );
        int start( -1 );
        uint length( 0 );

        while( pos >= 0 )
            {
             pos = rx.indexIn( in, pos );
             if( pos >= 0 )
                {
                start = pos;
                length = rx.matchedLength();
                pos += rx.matchedLength();
                }
            }

        if( start >= 0 )
            {
            _hasNumber = true;
            _pre = in.mid( 0, start );
            _padding = length;
            _post = in.mid( start+length );
            }
        }
    }

XString XFileSequence::niceFilename() const
    {
    return _pre + "X" + _post;
    }

XString XFileSequence::parsedFilename( ) const
    {
    if( _pre != "" || _post != "" )
        {
        if( _hasNumber )
            {
            return _pre + "$" + XString::number( _padding ) + "F" + _post;
            }
        return _pre + _post;
        }
    return "";
    }

void XFileSequence::setHasFrameNumber( bool in )
    {
    _hasNumber = in;
    }

void XFileSequence::setPreNumberString( XString in )
    {
    _pre = in;
    }

void XFileSequence::setPadding( uint in )
    {
    _padding = in;
    }

void XFileSequence::setPostNumberString( XString in )
    {
    _post = in;
    }

bool XFileSequence::hasFrameNumber( ) const
    {
    return _hasNumber;
    }

XString XFileSequence::preNumberString( ) const
    {
    return _pre;
    }

uint XFileSequence::padding( ) const
    {
    return _padding;
    }

XString XFileSequence::postNumberString( ) const
    {
    return _post;
    }

XString XFileSequence::getFilename( int frame ) const
    {
    if( _hasNumber )
        {
        QString num( QString::number( frame ) );

        for( unsigned int x=num.length(); x<_padding; x++ )
            {
            num = "0" + num;
            }
        return _pre + num + _post;
        }
    return _pre + _post;
    }

