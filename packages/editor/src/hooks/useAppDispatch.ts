/**
 * Custom Redux Hooks
 * Typed hooks for use throughout the application
 */

import { useDispatch } from 'react-redux';
import type { AppDispatch } from '../store/store';

export const useAppDispatch = () => useDispatch<AppDispatch>();
