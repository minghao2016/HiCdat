// ==========================================================================
//                 SeqAn - The Library for Sequence Analysis
// ==========================================================================
// Copyright (c) 2006-2013, Knut Reinert, FU Berlin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of Knut Reinert or the FU Berlin nor the names of
//       its contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL KNUT REINERT OR THE FU BERLIN BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// ==========================================================================
// Author: David Weese <david.weese@fu-berlin.de>
// ==========================================================================

#ifndef SEQAN_HEADER_INDEX_QGRAM_OPENADRESSING_H
#define SEQAN_HEADER_INDEX_QGRAM_OPENADRESSING_H

namespace SEQAN_NAMESPACE_MAIN
{

	struct OpenAddressing_;
	typedef Tag<OpenAddressing_> OpenAddressing;
	
	template <typename THashValue>
	struct BucketMap 
	{
		String<THashValue>	qgramCode;
	};

	// use the index value type as shape value type
	template < typename TObject, typename TShapeSpec >
	struct Fibre< Index<TObject, IndexQGram<TShapeSpec, OpenAddressing> >, FibreBucketMap>
	{
		typedef typename Fibre< Index<TObject, IndexQGram<TShapeSpec, OpenAddressing> >, FibreShape>::Type TShape;
		typedef typename Value<TShape>::Type	THashValue;
		typedef BucketMap<THashValue>			Type;
	};
	
/**
.Spec.OpenAddressing
..summary:An index based on an array of sorted q-grams.
..cat:Index
..general:Spec.IndexQGram
..signature:Index<TText, IndexQGram<TShapeSpec, OpenAddressing> >
..param.TText:The text type.
...type:Class.String
..param.TShapeSpec:The @Class.Shape@ specialization type.
...note:This can be either a $TSpec$ argument (e.g. $SimpleShape$) or a complete @Class.Shape@ class (e.g. Shape<Dna, SimpleShape>).
..remarks:This index uses a non-trivial hashing for mapping q-gram hash values to buckets.
This reduces the sizes of bucket directories (QGramDir, QGramCountsDir fibres) from |\Sigma|^q to min(\alpha*n,|\Sigma|^q), for a load factor \alpha>1.
A bucket still stores occurrences (or counts) of the same q-gram, but in contrast to the @Spec.IndexQGram@ index, buckets are in random order due to the hashing.
..include:seqan/index.h
.Memvar.OpenAddressing#alpha
..summary:Load factor. Controls space/time-tradeoff and must be greater 1. Default value is 1.6.
..class:Spec.OpenAddressing
*/	

#ifdef PLATFORM_WINDOWS_VS
#pragma warning( push )
// Disable warning C4521 locally (multiple copy constructors).
#pragma warning( disable: 4521 )
#endif  // PLATFORM_WINDOWS_VS

	template < typename TObject, typename TShapeSpec >
	class Index<TObject, IndexQGram<TShapeSpec, OpenAddressing> >
	{
    private:
        static const double defaultAlpha;
	public:
		typedef typename Fibre<Index, QGramText>::Type		TText;
		typedef typename Fibre<Index, QGramSA>::Type		TSA;
		typedef typename Fibre<Index, QGramDir>::Type		TDir;
		typedef typename Fibre<Index, QGramCounts>::Type	TCounts;
		typedef typename Fibre<Index, QGramCountsDir>::Type	TCountsDir;
		typedef typename Fibre<Index, QGramShape>::Type		TShape;
		typedef typename Fibre<Index, QGramBucketMap>::Type	TBucketMap;
		typedef typename Cargo<Index>::Type					TCargo;
		typedef typename Size<Index>::Type					TSize;

		Holder<TText>	text;		// underlying text
		TSA				sa;			// suffix array sorted by the first q chars
		TDir			dir;		// bucket directory
		TCounts			counts;		// counts each q-gram per sequence
		TCountsDir		countsDir;	// directory for count buckets
		TShape			shape;		// underlying shape
		TCargo			cargo;		// user-defined cargo
		TBucketMap		bucketMap;	// bucketMap table (used by open-addressing index)
		TSize			stepSize;	// store every <stepSize>'th q-gram in the index

		double			alpha;		// for m entries the hash map has at least size alpha*m

		Index():
			stepSize(1),
			alpha(defaultAlpha) {}

		Index(Index &other):
			text(other.text),
			sa(other.sa),
			dir(other.dir),
			counts(other.counts),
			countsDir(other.countsDir),
			shape(other.shape),
			cargo(other.cargo),
			stepSize(1),
			alpha(defaultAlpha) {}

		Index(Index const &other):
			text(other.text),
			sa(other.sa),
			dir(other.dir),
			counts(other.counts),
			countsDir(other.countsDir),
			shape(other.shape),
			cargo(other.cargo),
			stepSize(1),
			alpha(defaultAlpha) {}

		template <typename TText_>
		Index(TText_ &_text):
			text(_text),
			stepSize(1),
			alpha(defaultAlpha) {}

		template <typename TText_>
		Index(TText_ const &_text):
			text(_text),
			stepSize(1),
			alpha(defaultAlpha) {}

		template <typename TText_, typename TShape_>
		Index(TText_ &_text, TShape_ const &_shape):
			text(_text),
			shape(_shape),
			stepSize(1),
			alpha(defaultAlpha) {}

		template <typename TText_, typename TShape_>
		Index(TText_ const &_text, TShape_ const &_shape):
			text(_text),
			shape(_shape),
			stepSize(1),
			alpha(defaultAlpha) {}
	};
#ifdef PLATFORM_WINDOWS_VS
// Enable warning C4521 again (multiple copy operators).
#pragma warning( pop )
#endif  // PLATFORM_WINDOWS_VS


    template < typename TObject, typename TShapeSpec >
    const double Index<TObject, IndexQGram<TShapeSpec, OpenAddressing> >::defaultAlpha = 1.6;

	//////////////////////////////////////////////////////////////////////////////
	// Counting sort - Step 1: Clear directory
	template < typename TDir, typename THashValue >
	inline void _qgramClearDir(TDir &dir, BucketMap<THashValue> &bucketMap)
	{
        if (!empty(dir))
            arrayFill(begin(dir, Standard()), end(dir, Standard()), 0);
        if (!empty(bucketMap.qgramCode))
            arrayFill(begin(bucketMap.qgramCode, Standard()), end(bucketMap.qgramCode, Standard()), (THashValue)-1);
	}

    template < typename TBucketMap, typename TValue >
    inline TValue
    _hashFunction(TBucketMap const &, TValue val)
    {
        // WARNING:
        // As SSE4.2 is not always available, the bucket order is platform dependent
#ifdef __SSE4_2__
        return _mm_crc32_u64(0ul, val);
#else
        return ((val * 43) ^ (val >> 20)) + val;
#endif
    }
    

	template < typename THashValue, typename THashValue2 >
	inline THashValue
	requestBucket(BucketMap<THashValue> &bucketMap, THashValue2 code)
	{
		typedef unsigned long TSize;
		// get size of the index

		// check whether bucket map is disabled and
		// where the hash should be found if no collision took place before
		register TSize hlen = length(bucketMap.qgramCode);
		if (hlen == 0ul) return code;

        register TSize h1 = _hashFunction(bucketMap, code);
#ifdef SEQAN_OPENADDRESSING_COMPACT
        --hlen;
		h1 %= hlen;
#else
        hlen -= 2;
		h1 &= hlen;
#endif
		// was the entry empty?
		if (bucketMap.qgramCode[h1] == (THashValue)-1)
		{
			bucketMap.qgramCode[h1] = code;
			return h1;
		}
		// if not ...
		else
		{
			// is it occupied by our code?
			if (bucketMap.qgramCode[h1] == code)
				return h1;
            
			// if not we have a collision -> probe for our code or an empty entry
            //
            // do linear probing if we need to save memory (when SEQAN_OPENADDRESSING_COMPACT is defined)
            // otherwise do quadratic probing to avoid clustering (Cormen 1998)
            register TSize delta = 0;
            (void)delta;
            do {
#ifdef SEQAN_OPENADDRESSING_COMPACT
                h1 = (h1 + 1) % hlen;               // linear probing guarantees that all entries are visited
#else
                h1 = (h1 + delta + 1) & hlen;       // for power2-sized tables the (i*i+i)/2 probing guarantees the same
                ++delta;
#endif
            } while (bucketMap.qgramCode[h1] != (THashValue)-1 && bucketMap.qgramCode[h1] != code);
            bucketMap.qgramCode[h1] = code;
            return h1;
		}
	}

	template < typename THashValue, typename THashValue2 >
	inline THashValue
	getBucket(BucketMap<THashValue> const &bucketMap, THashValue2 code)
	{
		typedef unsigned long TSize;
		// get size of the index
		
		// check whether bucket map is disabled and
		// where the hash should be found if no collision took place before
		register TSize hlen = length(bucketMap.qgramCode);
		if (hlen == 0ul) return code;

        register TSize h1 = _hashFunction(bucketMap, code);
#ifdef SEQAN_OPENADDRESSING_COMPACT
        --hlen;
        h1 %= hlen;
#else
        hlen -= 2;
		h1 &= hlen;
#endif
		
        // probe for our code or an empty entry
        //
        // do linear probing if we need to save memory (when SEQAN_OPENADDRESSING_COMPACT is defined)
        // otherwise do quadratic probing to avoid clustering (Cormen 1998)
        register TSize delta = 0;
        (void)delta;
		while (bucketMap.qgramCode[h1] != code && bucketMap.qgramCode[h1] != (THashValue)-1)
		{
#ifdef SEQAN_OPENADDRESSING_COMPACT
			h1 = (h1 + 1) % hlen;               // linear probing guarantees that all entries are visited
#else
			h1 = (h1 + delta + 1) & hlen;       // for power2-sized tables the (i*i+i)/2 probing guarantees the same
            ++delta;
#endif
		}
		return h1;
	}
    

	template <typename TObject, typename TShapeSpec>
	inline __int64 _fullDirLength(Index<TObject, IndexQGram<TShapeSpec, OpenAddressing> > const &index) 
	{
		typedef Index<TObject, IndexQGram<TShapeSpec, OpenAddressing> >	TIndex;
		typedef typename Fibre<TIndex, QGramDir>::Type						TDir;
		typedef typename Fibre<TIndex, FibreShape>::Type					TShape;
		typedef typename Host<TShape>::Type									TTextValue;
		typedef typename Value<TDir>::Type									TDirValue;
		typedef typename Value<TShape>::Type								THashValue;
		
		double num_qgrams = _qgramQGramCount(index) * index.alpha;
		double max_qgrams = pow((double)ValueSize<TTextValue>::VALUE, (double)weight(indexShape(index)));
		__int64 qgrams;
		
		// compare size of open adressing with 1-1 mapping and use the smaller one
		if (num_qgrams * (sizeof(TDirValue) + sizeof(THashValue)) < max_qgrams * sizeof(TDirValue))
		{
			qgrams = (__int64)ceil(num_qgrams);
#ifndef SEQAN_OPENADDRESSING_COMPACT
			__int64 power2 = 1;
			while (power2 < qgrams)
				power2 <<= 1;
			qgrams = power2;
#endif
			resize(const_cast<TIndex &>(index).bucketMap.qgramCode, qgrams + 1, Exact());
		} else
		{
			qgrams = (__int64)ceil(max_qgrams);
			clear(const_cast<TIndex &>(index).bucketMap.qgramCode);	// 1-1 mapping, no bucket map needed
		}
		
		return qgrams + 1;
	}
	
}

#endif //#ifndef SEQAN_HEADER_...
