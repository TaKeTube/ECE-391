typedef struct martian_english_message_lock {
    // The type of synchronization primitive you may use is {\fix spinlock\_t}.
    // You may add up to 3 elements to this struct.
    spinlock_t lock;
    volatile unsigned int engNum;
    volatile unsigned int marNum;
} me_lock;

void melon_input(me_lock* lock, msg* message) {
    unsigned int flags;
    if(lock == NULL) return;
    while(1){
        spin_lock_irqsave(lock->lock, flags);
        if(lock->engNum == 0 && lock->marNum < 4){
            lock->marNum++;
            translate_to_martian(message);
            spin_unlock_irqstore(lock->lock, flags);
            break;
        }
        spin_unlock_irqstore(lock->lock, flags);
    }
}

void martian_input(me_lock* lock, msg* message) {
    unsigned int flags;
    if(lock == NULL) return;
    while(1){
        spin_lock_irqsave(lock->lock, flags);
        if(lock->engNum < 10){
            lock->engNum++;
            translate_to_english(message);
            spin_unlock_irqstore(lock->lock, flags);
            break;
        }
        spin_unlock_irqstore(lock->lock, flags);
    }
}

int melon_get_output(me_lock* lock, msg* message) {
    unsigned int flags;
    if(lock == NULL) return -1;
    spin_lock_irqsave(lock->lock, flags);
    if(lock->engNum > 0){
        lock->engNum--;
        get_translation_in_english(message);
    }else{
        spin_unlock_irqstore(lock->lock, flags);
        return -1;
    }
    spin_unlock_irqstore(lock->lock, flags);
    return 0;
}

int martian_get_output(me_lock* lock, msg* message) {
    unsigned int flags;
    if(lock == NULL) return -1;
    spin_lock_irqsave(lock->lock, flags);
    if(lock->marNum > 0){
        lock->marNum--;
        get_translation_in_martian(message);
    }else{
        spin_unlock_irqstore(lock->lock, flags);
        return -1;
    }
    spin_unlock_irqstore(lock->lock, flags);
    return 0;
}

/* 
Use these four routines to interface to the translation hardware and
queueing system.  Note that the translate_to routines do not check for
full output queues, nor do the get_transation routines check for 
empty queues.  None of the four routines should be called simultaneously
with any others (including themselves).

You do not need to define these functions, but you need to call them in 
your synchronization interface.
*/

/* 
 * translate_to_english
 *   DESCRIPTION: translate the message in the buffer to English;
 *      then put the message in the corresponding queue.
 *   INPUTS: message - a pointer to the input message buffer.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECT: none
 */
void translate_to_english(msg* message);

/* 
 * translate_to_martian
 *   DESCRIPTION: translate the message in the buffer to Martian;
 *      then put the message in the corresponding queue.
 *   INPUTS: message - a pointer to the input message buffer.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECT: none
 */
void translate_to_martian(msg* message);

/* 
 * get_translation_in_english
 *   DESCRIPTION: get a translated message in English.
 *   INPUTS: message - a pointer to the message buffer which will
 *      be filled in with a translated English message.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECT: Will fill in the given message buffer.
 */
void get_translation_in_english(msg* message);

/* 
 * get_translation_in_martian
 *   DESCRIPTION: get a translated message in Martian.
 *   INPUTS: message - a pointer to the message buffer which will
 *      be filled in with a translated Martian message.
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECT: Will fill in the given message buffer.
 */
void get_translation_in_martian(msg* message);
