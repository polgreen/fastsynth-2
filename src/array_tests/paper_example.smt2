(set-logic ALL)

(declare-fun memory () (Array (_ BitVec 32) (_ BitVec 32)))
(declare-fun memory! () (Array (_ BitVec 32) (_ BitVec 32)))

(declare-fun cache_address () (_ BitVec 32))
(declare-fun cache_address! () (_ BitVec 32))
(declare-fun cache_data () (_ BitVec 32))
(declare-fun cache_data! () (_ BitVec 32))
(declare-fun read_address () (_ BitVec 32))
(declare-fun read_address! () (_ BitVec 32))

;mem is initialized to hold arbitrary data values at each address.cache is initialized to hold an invalid address,0x00000000, with an arbitrary data value

(define-fun init-fn ((memory (Array (_ BitVec 32) (_ BitVec 32))) (cache_address (_ BitVec 32))
(cache_data (_ BitVec 32))) Bool 
	(= cache_address (_ bv0 32)))


; address is updated with read address. If read address is same as cache address, cache data remains same. Otherwise cache data is updated with mem[addr].

(define-fun trans-fn ((memory (Array (_ BitVec 32) (_ BitVec 32)))(cache_address (_ BitVec 32))
(cache_data (_ BitVec 32)) (memory! (Array (_ BitVec 32) (_ BitVec 32)))(cache_address! (_ BitVec 32)) (cache_data! (_ BitVec 32))(read_address (_ BitVec 32))) Bool
(and (= cache_address! read_address)
(ite (= read_address cache_address) (= cache_data! cache_data) (= cache_data! (select memory read_address))) 
(= memory! memory)))
	

(define-fun inv-fn ((memory (Array (_ BitVec 32) (_ BitVec 32))) (cache_address (_ BitVec 32))
(cache_data (_ BitVec 32))(read_address (_ BitVec 32))) Bool 
	(forall ((x (_ BitVec 32))) 
	(=> (= x read_address)
	(=> (and (= cache_address read_address) (not (= cache_address (_ bv0 32))))
	(= cache_data (select memory read_address))))))


(assert (not (=> (and (inv-fn memory cache_address cache_data read_address) 
(trans-fn memory cache_address cache_data memory! cache_address! cache_data! read_address)) (inv-fn memory! cache_address! cache_data! read_address!))))
(check-sat)

;(assert (not (and (=> (init-fn memory cache_address cache_data) (inv-fn memory cache_address cache_data read_address))
;(=> (and (inv-fn memory cache_address cache_data read_address) 
;(trans-fn memory cache_address cache_data memory! cache_address! cache_data! read_address)) (inv-fn memory! cache_address! cache_data! read_address!)))))


